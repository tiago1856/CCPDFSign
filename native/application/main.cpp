// ////////////////////////////////////////////////////////////////////////////
// PDF Signature with the Portuguese Identity Card - for internal use only
// ////////////////////////////////////////////////////////////////////////////
// 
// File:    native_1.cpp
// Author:  John Doe
// Date:    2022.01.13
// Remark:  This console program is used to process messages sent to and from Chromium 
//          extension.
//
// Copyright 2022 All Rights Reserved.
// 
// ////////////////////////////////////////////////////////////////////////////
// $(ProjectName)

//#define DEBUG   // uncomment line for logging

#include <iostream>
#include <fstream>
#include <json/json.h>
#include <stdio.h>
#include <filesystem>


#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
#endif

#include "base64.h"
#include "cc.h"
#ifdef DEBUG
    #include "log.h"
    #define LOG(str) log(str)
#else
    #define LOG(str)
#endif

const int MAX_BLOCK_SIZE = 800000;  // max size of any message block
                                    // consider max size as 800k, because yes
const std::string FILENAME_2_SIGN = "temp_doc_2_sign.pdf";
const std::string FILENAME_SIGNED = "temp_doc_SIGNED.pdf";

using namespace std;




// Define union to read the message size easily
typedef union {
    unsigned long u32;
    unsigned char u8[4];
} U32_U8;


int SetBinaryMode(FILE* file)
{
    int result;

    result = _setmode(_fileno(file), _O_BINARY);
    if (result == -1)
    {
        perror("Cannot set mode");
        return result;
    }
    // set do not use buffer
    result = setvbuf(file, NULL, _IONBF, 0);
    if (result != 0)
    {
        perror("Cannot set zero buffer");
        return result;
    }

    return 0;
}


void sendMessage(const string& key, const string& _message)
{
    string message = "{\"" + key + "\": \"" + _message + "\"}";;
    // Collect the length of the message
    size_t len = message.length();
    // We need to send the 4 bytes of length information
    cout << char(((len >> 0) & 0xFF))
        << char(((len >> 8) & 0xFF))
        << char(((len >> 16) & 0xFF))
        << char(((len >> 24) & 0xFF));
    // Now we can output our message
    cout << message;
}


int main(int argc, char* argv[]) 
{
    bool error = false;
    if (SetBinaryMode(stdin) != 0)
    {
        return -1;
    }
    if (SetBinaryMode(stdout) != 0)
    {
        return -1;
    }
        // the files will be saved in a temporay directory. Ex: C:\Users\xxx\AppData\Local\Temp
        auto temp_path = std::filesystem::temp_directory_path().string();
       // std::string temp_path = std::string(_temp.begin(), _temp.end());
        size_t iSize = 0;
        U32_U8 lenBuf;
        lenBuf.u32 = 0;
        char* jsonMsg = NULL; // a json message encoded in utf-8 
        sendMessage("status", "READY");


        iSize = fread(lenBuf.u8, 1, 4, stdin);
        sendMessage("status", "READING");

        if (iSize == 4)
        {
            int iLen = (int)lenBuf.u32;
            // now read the message
            if (iLen > 0)
            {
                jsonMsg = (char*)malloc(sizeof(char) * iLen);// 8 * iLen
                iSize = fread(jsonMsg, 1, iLen, stdin);

                // process the message {data, pos_x, pos_x, page}           
                JSONCPP_STRING err;
                Json::Value obj;
                string rawJson = jsonMsg;
                Json::CharReaderBuilder builder;
                const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
                reader->parse(rawJson.c_str(), rawJson.c_str() + rawJson.length(), &obj, &err);
                string data = obj["data"].asString();
                double pos_x = obj["pos_x"].asDouble();
                double pos_y = obj["pos_y"].asDouble();
                int page = obj["page"].asInt();

                // save file in computer
                string result = base64_decode(data, false);
                ofstream wf((temp_path + FILENAME_2_SIGN).c_str(), ios::binary);
                wf << result;
                wf.close();
            
                // -------------------------
                // SIGN SAVED FILE
                // TODO: put this section in a function inside cc.cpp
                // -------------------------
                LOG("Initing EID SDK ...");
                eIDMW::PTEID_InitSDK();
                LOG("EID SDK initiated!");
                sendMessage("status", "SDK READY");
			    try {
                    LOG("Signing file ...");
                    sendMessage("status", "SIGNING");
				    signPDFFile(1, pos_x, pos_y, "", "", (temp_path + FILENAME_2_SIGN).c_str(), (temp_path + FILENAME_SIGNED).c_str());
                    LOG("File signed!");
                    sendMessage("status", "SIGNED");
                } 
			    catch (eIDMW::PTEID_Exception ex) {
                    std::string error_msg = errorMessage(ex.GetError(), ex.GetMessage());
                    LOG("Error: " + error_msg);
                    sendMessage("error", error_msg);
                    error = true;
			    }
                catch (...)
                {
                    sendMessage("error", "UNKNOWN ERROR");
                    LOG("UNKNOWN ERROR");
                }
                LOG("Releasing EID SDK ...");
                eIDMW::PTEID_ReleaseSDK();
                LOG("EID SDK released!");

                // -----------------------------
                // SEND SIGNED FILE TO BROWSER
                // -----------------------------
                // read temporary signed file
                if (!error) 
                {
                    ifstream in((temp_path + FILENAME_SIGNED).c_str(), ios::binary);
                    in.seekg(0, ios::end);
                    std::streamoff iSize = in.tellg();
                    in.seekg(0, ios::beg);

                    char* pBuff = new char[iSize];
                    memset(pBuff, 0, sizeof(pBuff));
                    in.read(pBuff, iSize);
                    in.close();

                    std::string xxx(pBuff, pBuff + iSize);
                    string data2 = base64_encode(xxx, false);
 


                    // i don't feel like using buffers and pointers so let's use strings for everything
                    // length of the message
                    size_t total_size = data2.length();
                    // total number of blocks
                    size_t blocks = (total_size / MAX_BLOCK_SIZE) + 1;
                    size_t desc = 0;
                    // divide the mesage into blocks
                    for (unsigned int i = 0; i < blocks; i++)
                    {
                        // block size
                        size_t delta = (i < blocks - 1) ? MAX_BLOCK_SIZE : total_size - MAX_BLOCK_SIZE * (blocks - 1);
                        // data to send
                        string block_message = data2.substr(desc, delta);
                        // create json with the data
                        string message_2_send = "{\"data\": \"" + block_message + "\"}";
                        desc += delta;               
                        //sendMessage("info", to_string(delta));
                        size_t block_size = message_2_send.length();
                        //send the 4 bytes of length information
                        cout << char(((block_size >> 0) & 0xFF))
                            << char(((block_size >> 8) & 0xFF))
                            << char(((block_size >> 16) & 0xFF))
                            << char(((block_size >> 24) & 0xFF));
                        // output our message
                        cout << message_2_send;
                
                    }

                    //sendMessage("info", to_string((iLen / BLOCK_SIZE)));
                    // all data sent
                    sendMessage("status", "DONE");
                    LOG("File processing completed succesfully!");
                }
            }
            else
            {
                sendMessage("error", "Nothing to process!");
                LOG("Nothing to process!");
            }

            // free resource
            if (jsonMsg != NULL) free(jsonMsg);

            // Prevent "Native host has exited." error
           // string temp;
           // cin >> temp;
        }
        else
        {
            sendMessage("error", "Something wrong with the message length specification!");
            LOG("Something wrong with the message length specification!");
        }
        /*
        if (remove(FILENAME_2_SIGN) != 0 && remove(FILENAME_SIGNED) != 0)
        {
            log("Error deleting Temporary files");
        }
        else
        {
            log("Temporary files successfully deleted");
        }
        */
 
    sendMessage("status", "Operation Completed!");
    LOG("--------- OPERATION COMPLETED -------------");

    // Prevent "Native host has exited." error
    string temp;
    cin >> temp;

    return 0;   

}

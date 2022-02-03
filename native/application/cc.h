#pragma once

#pragma comment(lib, "C:\\Users\\guard\\source\\repos\\native_1\\sdk\\pteidlib.lib")

#include "eidlib.h"
#include "eidErrors.h"
#include "eidlibException.h"

//using namespace eIDMW;

/**
 * Lista de c�digos de erro do middleware
 *
 * @param error_code c�digo de erro
 */
const std::string errorMessage(int error_code, const std::string& extra_msg);


/**
* Devolve o objecto de contexto do primeiro leitor com cart�o que for encontrado no sistema.
* Alternativamente caso n�o existam cart�es inseridos devolver� o primeiro leitor que encontrar no sistema.
*/
eIDMW::PTEID_ReaderContext& getFirstCard();

/*
* Sign a PDF file.
*/
bool signPDFFile(
    int page,
    double coord_x,
    double coord_y,
    const char* location,
    const char* reason,
    const char* infile_path,
    const char* outfile_path
);
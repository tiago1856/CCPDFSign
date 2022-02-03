
#include "cc.h"

const std::string errorMessage(int error_code, const std::string& extra_msg) {
    std::string message;
    switch (error_code) {
    case EIDMW_ERR_CARD:
        message = ("Erro desconhecido - Problemas com o serviço de leitor de cartões.");
        break;
    case EIDMW_ERR_PIN_BAD:
        message = ("Pin inválido, não tente novamente.");
        break;
    case EIDMW_ERR_NOT_AUTHENTICATED:
        message = ("Não autenticado. PIN não especificado.");
        break;
    case EIDMW_ERR_NOT_SUPPORTED:
        message = ("Comando não suportado por este cartão.");
        break;
    case EIDMW_ERR_PIN_BLOCKED:
        message = ("PIN bloqueado.");
        break;
    case EIDMW_ERR_NO_CARD:
        message = ("Cartão removido ou não presente.");
        break;
    case EIDMW_ERR_FILE_NOT_FOUND:
        message = ("Ficheiro não encontrado.");
        break;
    case EIDMW_ERR_CARD_COMM:
        message = ("Erro na comunicação com o cartão.");
        break;
    case EIDMW_ERR_NO_READER:
        message = ("Leitor não encontrado.");
        break;
    case EIDMW_ERR_CANT_CONNECT:
        message = ("Cartão presente, mas não é possivel acede-lo.");
        break;
    case EIDMW_ERR_CARD_SHARING:
        message = ("Impossivel aceder ao cartão.");
        break;
    case EIDMW_ERR_INCOMPATIBLE_READER:
        message = ("Leitor incompativel.");
        break;
    case EIDMW_TIMESTAMP_ERROR:
        message = ("[DEV::ERROR] Erro relacionado com os parâmetros fornecidos.");
        break;
    default:
        message = ("Erro desconhecido. ", extra_msg);
        break;
    }

    return message;
}



eIDMW::PTEID_ReaderContext& getFirstCard() {
    return eIDMW::PTEID_ReaderSet::instance().getReader();
}


bool signPDFFile(
    int page,
    double coord_x,
    double coord_y,
    const char* location,
    const char* reason,
    const char* infile_path,
    const char* outfile_path
)
{
    eIDMW::PTEID_ReaderContext& context = getFirstCard();
    if (context.isCardPresent())
    {
        eIDMW::PTEID_EIDCard& card = context.getEIDCard();
        eIDMW::PTEID_PDFSignature signature(infile_path);
        signature.setSignatureLevel(eIDMW::PTEID_SignatureLevel::PTEID_LEVEL_TIMESTAMP);
        card.SignPDF(signature, page, coord_x, coord_y, location, reason, outfile_path);
        return true;
    }
    return false;
}


#include "ErrorLib.h"

ErrorLib ERRORS_LIST;

ErrorLib::ErrorLib() {}
ErrorLib::~ErrorLib() {}

void ErrorLib::addError(ErrorCode error) {
    if (error < NONE || error >= ERROR_TYPE_COUNT) {
        return;
    }

    for (const auto& err : _errors) {
        if (err == error) {
            return;
        }
    }

    _errors.push_back(error);
}

void ErrorLib::clearErrors() {
    _errors.clear();
}

void ErrorLib::clearError(ErrorCode error) {
    for (auto it = _errors.begin(); it != _errors.end(); ) {
        if (*it == error) {
            it = _errors.erase(it);
        }
        else {
            ++it;
        }
    }
}

bool ErrorLib::findError(ErrorCode error) {
    for (int i = 0; i < _errors.size(); i++) {
        if (_errors[i] == error) {
            return true;
        }
    }
    return false;
}

void ErrorLib::printErrors() {
    for (int i = 0; i < _errors.size(); i++) {
        LOG_ERROR("Error code: %s", _errors[i]);
    }
}

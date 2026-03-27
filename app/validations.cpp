#include "validations.h"

bool isValidName(const string& name) {
    if (name.empty()) {
        return false;
    }
    for (char character : name) {
        if (character != ' ') {
            return true;
        }
    }
    return false;
}

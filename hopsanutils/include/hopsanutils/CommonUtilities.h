/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   CommonUtilities.h
//! @brief Contains common utility functions
//!
//$Id$

#ifndef COMMONUTILITIES_H
#define COMMONUTILITIES_H

#include <string>
#include <vector>
#include <algorithm>

namespace hopsan {

// ===== Help Functions for String Paths =====
void splitFilePath(const std::string &fullPath, std::string &rBasePath, std::string &rFileName);
void splitFileName(const std::string &fileName, std::string &rBaseName, std::string &rExtension);
void splitStringOnDelimiter(const std::string &string, char delim, std::vector<std::string> &rSplitVector);
std::string relativePath(std::string basePath, std::string fullPath);

// ===== Template Help Function =====
template <typename ContainerT, typename ValueT>
bool contains(const ContainerT& container, const ValueT& value) {
    auto it = std::find(container.begin(), container.end(), value);
    return (it != container.end());
}

struct RVMessage {
    enum class MessageType {Info, Warning, Error, Debug};
    RVMessage(MessageType mt, const std::string &t) : type(mt), text(t) {}
    MessageType type;
    std::string text;
};

template <typename ReturnValueType = bool>
class RV {
public:
    RV() : mIsValid(false) {}
    RV(bool isValid) : mIsValid(isValid) {}
    //! @todo In case ReturnValueType = bool, also set rv
    RV(bool isValid, const ReturnValueType &rv) : mIsValid(isValid), mRv(rv) {}
    void addInfo(const std::string &message) {
        mMessages.emplace_back(RVMessage::MessageType::Info, message);
    }
    void addWarning(const std::string &message) {
        mMessages.emplace_back(RVMessage::MessageType::Warning, message);
    }
    void addError(const std::string &message) {
        mMessages.emplace_back(RVMessage::MessageType::Error, message);
    }
    void addDebug(const std::string &message) {
        mMessages.emplace_back(RVMessage::MessageType::Debug, message);
    }

    RV<ReturnValueType>& fail() {
        mIsValid = false;
        return *this;
    }
    RV<ReturnValueType>& fail(const std::string &errorMessage) {
        mIsValid = false;
        addError(errorMessage);
        return *this;
    }
    RV<ReturnValueType>& success() {
        mIsValid = true;
        return *this;
    }
    RV<ReturnValueType>& success(ReturnValueType rv) {
        mIsValid = true;
        mRv = rv;
        return *this;
    }

    bool operator()() const {
        return mIsValid;
    }

    ReturnValueType& rv() {
        return mRv;
    }

    const std::vector<RVMessage>& messages() {
        return mMessages;
    }

    void copyMessages(const std::vector<RVMessage> &messages) {
        for (const auto& msg : messages) {
            mMessages.push_back(msg);
        }
    }

protected:
    bool mIsValid;
    ReturnValueType mRv;
    std::vector<RVMessage> mMessages;
};


}

#endif // COMMONUTILITIES_H

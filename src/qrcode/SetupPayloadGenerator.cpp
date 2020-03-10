/*
 *
 *    <COPYRIGHT>
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file implements a QRCode Setup Payload generator in accordance
 *      with the CHIP specification.
 *
 */

#include "SetupPayloadGenerator.h"
#include "SetupCodeUtils.h"

#include <iostream>
#include <vector>

using namespace chip;
using namespace std;

void SetupPayloadGenerator::resetBitSet()
{
    mPayloadBitsIndex = kTotalPayloadDataSizeInBits;
    mPayloadBits.reset();
}

// Populates numberOfBits starting from LSB of input into mPayloadBits
void SetupPayloadGenerator::populateInteger(uint64_t input, size_t numberOfBits)
{
    int numberOfBitsModified = 0;

    mPayloadBitsIndex -= numberOfBits;
    int currentIndex = mPayloadBitsIndex;

    int endIndex = currentIndex + (numberOfBits - 1);

    int itercount = 1;
    while (currentIndex <= endIndex) {
        itercount++;
        input & 1 ? mPayloadBits.set(currentIndex) : mPayloadBits.reset(currentIndex);
        currentIndex++;
        input /= 2;
    }
}

void SetupPayloadGenerator::populateVersion()
{
    populateInteger(mPayload.version, kVersionFieldLengthInBits);
}

void SetupPayloadGenerator::populateVendorID()
{
    populateInteger(mPayload.vendorID, kVendorIDFieldLengthInBits);
}

void SetupPayloadGenerator::populateProductID()
{
    populateInteger(mPayload.productID, kProductIDFieldLengthInBits);
}

void SetupPayloadGenerator::populateCustomFlowRequiredField()
{
    populateInteger(mPayload.requiresCustomFlow, kCustomFlowRequiredFieldLengthInBits);
}

void SetupPayloadGenerator::populateRendezVousInfo()
{
    populateInteger(mPayload.rendezvousInformation, kRendezvousInfoFieldLengthInBits);
}

void SetupPayloadGenerator::populateDiscriminator()
{
    populateInteger(mPayload.discriminator, kPayloadDiscriminatorFieldLengthInBits);
}

void SetupPayloadGenerator::populateSetupPIN()
{
    populateInteger(mPayload.setUpPINCode, kSetupPINCodeFieldLengthInBits);
}

void SetupPayloadGenerator::populateReservedField()
{
    populateInteger(0, kReservedFieldLengthInBits);
}

void SetupPayloadGenerator::generateBitSet()
{
    resetBitSet();
    populateVersion();
    populateVendorID();
    populateProductID();
    populateCustomFlowRequiredField();
    populateRendezVousInfo();
    populateDiscriminator();
    populateSetupPIN();
    populateReservedField();
}

string SetupPayloadGenerator::payloadBinaryRepresentation()
{
    generateBitSet();
    return mPayloadBits.to_string();
}

// This function assumes bits.size() % 8 == 0
// TODO: Can this method be written in a more elegant way?
vector<uint16_t> arrayFromBits(bitset<kTotalPayloadDataSizeInBits> bits)
{
    vector<uint16_t> resultVector;
    size_t numberOfBits = bits.size();
    size_t numberOfBytes = numberOfBits / 8;
    bool oddNumOfBytes = (numberOfBytes % 2) ? true : false;

    // Traversing in reverse, hence startIndex > endIndex
    int endIndex = 0;
    int startIndex = bits.size() - 1;

    /*
    Every 2 bytes (16 bits) of binary source data are encoded to 3 characters of the Base-45 alphabet.
    If an odd number of bytes are to be encoded, the remaining single byte will be encoded
    to 2 characters of the Base-45 alphabet.
    */
    if (oddNumOfBytes) {
         endIndex = 8;
    }

    while (startIndex > endIndex) {
        int currentIntegerIndex = startIndex;
        uint16_t result = 0;
        for (int i = currentIntegerIndex; i > currentIntegerIndex - 16; i--) {
            result = result << 1;
            result = result | bits.test(i);
        }
        resultVector.push_back(result);
        startIndex -= 16;
    }

    // If we have odd number of bytes append the last byte.
    if (oddNumOfBytes) {
        uint16_t result = 0;
        for (int i = 7; i >= 0 ; i--) {
            result = result << 1;
            result = result & bits.test(i);
        }
        resultVector.push_back(result);
    }
    return resultVector;
}

string SetupPayloadGenerator::payloadBase45Representation()
{
    generateBitSet();
    vector<uint16_t> integerArray = arrayFromBits(mPayloadBits);
    string result;
    for (int idx = 0; idx < integerArray.size(); idx++) {
        result += base45EncodedString(integerArray[idx], 3);
    }
    return result;
}

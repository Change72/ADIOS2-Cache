/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * DataManReader.h
 *
 *  Created on: Feb 21, 2017
 *      Author: wfg
 */

#ifndef ADIOS2_ENGINE_DATAMAN_DATAMANREADER_H_
#define ADIOS2_ENGINE_DATAMAN_DATAMANREADER_H_

#include <iostream> //std::cout << Needs to go

#include <DataMan.h>

#include "adios2/ADIOSConfig.h"
#include "adios2/capsule/heap/STLVector.h"
#include "adios2/core/Engine.h"
#include "adios2/utilities/format/bp1/BP1Writer.h"

namespace adios
{

class DataManReader : public Engine
{

public:
    /**
     * Constructor for dataman engine Reader for WAN communications
     * @param adios
     * @param name
     * @param accessMode
     * @param mpiComm
     * @param method
     * @param debugMode
     * @param nthreads
     */
    using json = nlohmann::json;
    DataManReader(ADIOS &adios, const std::string &name,
                  const std::string accessMode, MPI_Comm mpiComm,
                  const Method &method);

    virtual ~DataManReader() = default;

    /**
     * Set callback function from user application
     * @param callback function (get) provided by the user to be applied in
     * DataMan
     */
    void SetCallBack(std::function<void(const void *, std::string, std::string,
                                        std::string, Dims)>
                         callback);

    Variable<void> *InquireVariable(const std::string &name,
                                    const bool readIn = true);
    Variable<char> *InquireVariableChar(const std::string &name,
                                        const bool readIn = true);
    Variable<unsigned char> *InquireVariableUChar(const std::string &name,
                                                  const bool readIn = true);
    Variable<short> *InquireVariableShort(const std::string &name,
                                          const bool readIn = true);
    Variable<unsigned short> *InquireVariableUShort(const std::string &name,
                                                    const bool readIn = true);
    Variable<int> *InquireVariableInt(const std::string &name,
                                      const bool readIn = true);
    Variable<unsigned int> *InquireVariableUInt(const std::string &name,
                                                const bool readIn = true);
    Variable<long int> *InquireVariableLInt(const std::string &name,
                                            const bool readIn = true);
    Variable<unsigned long int> *InquireVariableULInt(const std::string &name,
                                                      const bool readIn = true);
    Variable<long long int> *InquireVariableLLInt(const std::string &name,
                                                  const bool readIn = true);
    Variable<unsigned long long int> *
    InquireVariableULLInt(const std::string &name, const bool readIn = true);
    Variable<float> *InquireVariableFloat(const std::string &name,
                                          const bool readIn = true);
    Variable<double> *InquireVariableDouble(const std::string &name,
                                            const bool readIn = true);
    Variable<long double> *InquireVariableLDouble(const std::string &name,
                                                  const bool readIn = true);
    Variable<std::complex<float>> *
    InquireVariableCFloat(const std::string &name, const bool readIn = true);
    Variable<std::complex<double>> *
    InquireVariableCDouble(const std::string &name, const bool readIn = true);
    Variable<std::complex<long double>> *
    InquireVariableCLDouble(const std::string &name, const bool readIn = true);

    /**
     * Not implemented
     * @param name
     * @param readIn
     * @return
     */
    VariableCompound *InquireVariableCompound(const std::string &name,
                                              const bool readIn = true);

    void Close(const int transportIndex = -1);

private:
    bool m_DoRealTime = false;
    DataMan m_Man;
    std::function<void(const void *, std::string, std::string, std::string,
                       Dims)>
        m_CallBack; ///< call back function

    void Init(); ///< calls InitCapsules and InitTransports based on Method,
                 /// called from constructor
    void InitTransports(); ///< from Transports

    std::string
    GetMdtmParameter(const std::string parameter,
                     const std::map<std::string, std::string> &mdtmParameters);

    template <class T>
    Variable<T> *InquireVariableCommon(const std::string name,
                                       const bool readIn)
    {
        std::cout << "I am hooked to the DataMan library\n";
        std::cout << "Hello DatamanReader from rank " << m_RankMPI << "\n";
        std::cout << "Trying to read variable " << name
                  << " from one of the variables coming from a WAN transport\n";

        // here read variable metadata (dimensions, type, etc.)...then create a
        // Variable like below:
        // Variable<T>& variable = m_ADIOS.DefineVariable<T>( m_Name + "/" +
        // name, )
        // return &variable; //return address if success
        return nullptr; // on failure
    }
};

} // end namespace

#endif /* ADIOS2_ENGINE_DATAMAN_DATAMANREADER_H_ */

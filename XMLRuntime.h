/*
    Project FreeAX25
    Copyright (C) 2015  tania@df9ry.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FREEAX25_XMLRUNTIME_H_
#define FREEAX25_XMLRUNTIME_H_

#include <Configuration.h>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <ostream>
#include <string>

// ---------------------------------------------------------------------------
// SO entry point.
// ---------------------------------------------------------------------------

extern "C" {
	/**
	 * Read the configuration file.
	 * @param filename Name of the configuration file.
	 * @param conf Reference to configuration object.
	 */
	void run(
			const std::string& filename,
			FreeAX25::Runtime::Configuration& conf);
}

namespace XMLIO {

// ---------------------------------------------------------------------------
// This class encapsulate the XML Runtime interface.
// ---------------------------------------------------------------------------

class XMLRuntime {
public:
	/**
	 * Constructor
	 * @param e Global Environment
	 */
	XMLRuntime();

	/**
	 * You can not copy a XMLRuntime.
	 * @param other Not used.
	 */
	XMLRuntime(const XMLRuntime& other) = delete;

	/**
	 * You can not move a XMLRuntime.
	 * @param other Not used.
	 */
	XMLRuntime(XMLRuntime&& other) = delete;

	/**
	 * Destructor
	 */
	~XMLRuntime();

	/**
	 * You can not assign a XMLRuntime.
	 * @param other Not used.
	 * @return Not used.
	 */
	XMLRuntime& operator=(const XMLRuntime& other) = delete;

	/**
	 * You can not assign a XMLRuntime.
	 * @param other Not used.
	 * @return Not used.
	 */
	XMLRuntime& operator=(XMLRuntime&& other) = delete;

	/**
	 * Read configuration from XML file
	 * @param filename Name of the XML file
	 * @param config Configuration object
	 */
	void read(
			const std::string& filename,
			FreeAX25::Runtime::Configuration& config);

private:
};

// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of XMLCh data to local code page for display.
// ---------------------------------------------------------------------------
class StrX
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    StrX(const XMLCh* const toTranscode)
	{
	    // Call the private transcoding method
	    fLocalForm = XERCES_CPP_NAMESPACE::XMLString::transcode(toTranscode);
	}

	~StrX()
	{
		XERCES_CPP_NAMESPACE::XMLString::release(&fLocalForm);
	}

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const char* localForm() const
    {
        return fLocalForm;
    }

    const std::string str() const
    {
    	return std::string(fLocalForm);
    }

private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLocalForm
    //      This is the local code page form of the string.
    // -----------------------------------------------------------------------
    char*   fLocalForm;
};

inline std::ostream& operator<<(std::ostream& target, const StrX& toDump)
{
    target << toDump.localForm();
    return target;
}

} /* namespace XMLIO */

#endif /* FREEAX25_XMLRUNTIME_H_ */

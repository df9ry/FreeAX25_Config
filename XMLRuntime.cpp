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

#include <ServerEndPoint.h>
#include <ClientEndPoint.h>
#include <Plugin.h>
#include <Instance.h>
#include <Setting.h>
#include <Environment.h>

#include "XMLRuntime.h"
#include "DOMTreeErrorReporter.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <exception>
#include <string>

using namespace std;
using namespace XERCES_CPP_NAMESPACE;

void run(const string& filename, FreeAX25::Runtime::Configuration& conf) {
	XMLIO::XMLRuntime rt{};
	rt.read(filename, conf);
}

namespace XMLIO {

static inline const XMLCh* toX(const string& s) {
	return XMLString::transcode(s.c_str());
}

static inline const string fmX(const XMLCh* x) {
	return string(XMLString::transcode(x));
}

XMLRuntime::XMLRuntime() { XMLPlatformUtils::Initialize(); }

XMLRuntime::~XMLRuntime() { XMLPlatformUtils::Terminate(); }

static void readSettings(
		const std::string& id,
		XERCES_CPP_NAMESPACE::DOMElement* element,
		FreeAX25::Runtime::UniquePointerDict<FreeAX25::Runtime::Setting>& settings);

static void readPlugins(
		const std::string& id,
		XERCES_CPP_NAMESPACE::DOMElement* element,
		FreeAX25::Runtime::UniquePointerDict<FreeAX25::Runtime::Plugin>& plugins);

static void readInstances(
		const std::string& id,
		XERCES_CPP_NAMESPACE::DOMElement* element,
		FreeAX25::Runtime::UniquePointerDict<FreeAX25::Runtime::Instance>& instances);

void XMLRuntime::read(const string& filename, FreeAX25::Runtime::Configuration& config) {
    XercesDOMParser parser;
    parser.setValidationScheme(XercesDOMParser::Val_Always);
    parser.setDoNamespaces(true);
    parser.setDoSchema(true);
    parser.setDoXInclude(true);
    parser.setHandleMultipleImports(true);
    parser.setValidationSchemaFullChecking(true);
    parser.setCreateEntityReferenceNodes(false);
    parser.setIncludeIgnorableWhitespace(false);

    DOMTreeErrorReporter errReporter;
    parser.setErrorHandler(&errReporter);

    parser.parse(filename.c_str());

    if (errReporter.getSawErrors())
    	throw exception();

    // Now read configuration from the DOM Tree:
    XERCES_CPP_NAMESPACE::DOMDocument* doc = parser.getDocument();
    assert(doc != nullptr);

    auto rootElement = doc->getDocumentElement();
    auto configName = rootElement->getAttribute(toX("name"));
    config.setId(fmX(configName));

    { // Get settings:
		auto nodeList = rootElement->getElementsByTagName(toX("Settings"));
		if (nodeList->getLength() > 0)
			readSettings(
					"",
					static_cast<DOMElement*>(nodeList->item(0)),
					config.settings);
    }

    { // Get plugins:
		auto nodeList = rootElement->getElementsByTagName(toX("Plugins"));
		if (nodeList->getLength() > 0)
			readPlugins(
					"",
					static_cast<DOMElement*>(nodeList->item(0)),
					config.plugins);
    }
}

static void readSettings(const string& id, DOMElement* element,
		FreeAX25::Runtime::UniquePointerDict<FreeAX25::Runtime::Setting>& settings)
{
	if (element == nullptr) return;
	auto nodeList = element->getElementsByTagName(toX("Setting"));
	for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
		auto settingElement = static_cast<DOMElement*>(nodeList->item(i));
		string name  = fmX(settingElement->getAttribute(toX("name")));
		string value = fmX(settingElement->getTextContent());
		FreeAX25::Runtime::env().logDebug(
				"Setting " + id + "/" + name + " = \"" + value + "\"");
		settings.insertNew(name, new FreeAX25::Runtime::Setting(name, value));
	} // end for //
}

static void readPlugins(const string& id, DOMElement* element,
		FreeAX25::Runtime::UniquePointerDict<FreeAX25::Runtime::Plugin>& plugins)
{
	if (element == nullptr) return;
	auto nodeList = element->getElementsByTagName(toX("Plugin"));
	for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
		auto pluginNode = static_cast<DOMElement*>(nodeList->item(i));
		string name  = fmX(pluginNode->getAttribute(toX("name")));
		auto _file = pluginNode->getAttribute(toX("file"));
		string file = (_file != nullptr) ? fmX(_file) : "";
		FreeAX25::Runtime::env().logDebug(
				"Define plugin " + id + "/" + name + "(" + file + ")");
		auto plugin = new FreeAX25::Runtime::Plugin(name, file);

	    { // Get settings:
			auto nodeList = pluginNode->getElementsByTagName(toX("Settings"));
			if (nodeList->getLength() > 0)
				readSettings(
						id + "/" + name,
						static_cast<DOMElement*>(nodeList->item(0)),
						plugin->settings);
	    }

	    { // Get instances:
			auto nodeList = pluginNode->getElementsByTagName(toX("Instances"));
			if (nodeList->getLength() > 0)
				readInstances(
						id + "/" + name,
						static_cast<DOMElement*>(nodeList->item(0)),
						plugin->instances);
	    }
		plugins.insertNew(name, plugin);
	} // end for //
}

static void readInstances(const string& id, DOMElement* element,
		FreeAX25::Runtime::UniquePointerDict<FreeAX25::Runtime::Instance>& instances)
{
	if (element == nullptr) return;
	auto nodeList = element->getElementsByTagName(toX("Instance"));
	for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
		auto instanceNode = static_cast<DOMElement*>(nodeList->item(i));
		string name  = fmX(instanceNode->getAttribute(toX("name")));
		FreeAX25::Runtime::env().logDebug(
				"Define instance " + id + "/" + name);
		auto instance = new FreeAX25::Runtime::Instance(name);

	    { // Get client endpoints:
			auto nodeList = instanceNode->getElementsByTagName(toX("ClientEndPoint"));
			for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
				auto instanceNode = static_cast<DOMElement*>(nodeList->item(i));
				string name = fmX(instanceNode->getAttribute(toX("name")));
				string url  = fmX(instanceNode->getAttribute(toX("url")));
				FreeAX25::Runtime::env().logDebug(
						"Define client endpoint " + id + "/" + name + " as " + url);
				auto endpoint = new FreeAX25::Runtime::ClientEndPoint(name, url);
				instance->clientEndPoints.insertNew(name, endpoint);
			} // end for //
	    }

	    { // Get server endpoints:
			auto nodeList = instanceNode->getElementsByTagName(toX("ServerEndPoint"));
			for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
				auto instanceNode = static_cast<DOMElement*>(nodeList->item(i));
				string name = fmX(instanceNode->getAttribute(toX("name")));
				string url  = fmX(instanceNode->getAttribute(toX("url")));
				FreeAX25::Runtime::env().logDebug(
						"Define server endpoint " + id + "/" + name + " as " + url);
				auto endpoint = new FreeAX25::Runtime::ServerEndPoint(name, url);
				instance->serverEndPoints.insertNew(name, endpoint);
			} // end for //
	    }

	    { // Get settings:
			auto nodeList = instanceNode->getElementsByTagName(toX("Settings"));
			if (nodeList->getLength() > 0)
				readSettings(
						id + "/" + name,
						static_cast<DOMElement*>(nodeList->item(0)),
						instance->settings);
	    }

		instances.insertNew(name, instance);
	} // end for //
}

} /* namespace XMLIO */

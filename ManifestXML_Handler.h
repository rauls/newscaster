// ManifestXML_Handler.h: interface for the ManifestXML_Handler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MANIFESTXML_HANDLER_H__483CC994_6335_4F9C_AC9B_07386E306525__INCLUDED_)
#define AFX_MANIFESTXML_HANDLER_H__483CC994_6335_4F9C_AC9B_07386E306525__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "xercesc/sax2/DefaultHandler.hpp"
#include <afxtempl.h>
#include "ManifestInfo.h"

XERCES_CPP_NAMESPACE_USE

enum ManifestState              // all possible states of manifest 
{                               // state machine
   FINDING_FILE,
   FOUND_FILE,
};                


enum ManifestCharacterState     // all possible states of characters 
{                               // state machine
	MANIFEST_IMAGE,
	MANIFEST_MULTIMEDIA,
	MANIFEST_PRINT,
	MANIFEST_INVALID

};
class ManifestXML_Handler : public DefaultHandler 
{
public:
	ManifestXML_Handler(CEdit* debugEdit,           // See C++ implementation
					    CList <ManifestData, ManifestData>* manifestList);

	void Reset_Handler(CEdit* debugEdit,           // See C++ implementation
					    CList <ManifestData, ManifestData>* manifestList);

	virtual ~ManifestXML_Handler();                 // See C++ implementation


	void startElement(                              // See C++ implementation
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attrs
    );

	void endElement(                                // See C++ implementation
		const	XMLCh* const,
		const XMLCh* const,
		const XMLCh* const
	);

    void fatalError(const SAXParseException&);      // See C++ implementation

	void characters(const   XMLCh* const    chars,  // See C++ implementation
					const unsigned int    length);

private:
	CEdit* m_OutputEdit;                    // Edit Component used for 
                                            // display during debug

   	/* Flags that are used to establish if a complete
       manifest has been encountered */

	bool gotFileMethod;
	bool gotDateRequired;
	bool gotImageFile;
	bool gotImageFileType;
	bool gotMultimediaFile;
	bool gotMultimediaFileType;
	bool gotPrintFile;
	bool gotPrintFileType;


	ManifestState ManifestXML_State;        // current state of manifest 
                                            // state machine

    ManifestCharacterState XML_CharState;   // current state of charecters
                                            // state machine

	// list to hold complete manifests as they are found
    CList <ManifestData, ManifestData>* pManifestList;
	
	// structure to hold elements of the manifest as they are found
    ManifestData currentManifest;

   	void InitCurrentManifest();             // See C++ implementation

	void Debug(CString debugMessage);       // See C++ implementation


};

#endif // !defined(AFX_MANIFESTXML_HANDLER_H__483CC994_6335_4F9C_AC9B_07386E306525__INCLUDED_)

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
\ win_adapter.h,  Adapters to redirect a C-style callback into a member fn
/				  and wrappers for common Win32 API calls that uses callbacks
\ 
/ Version:	1.0, 2001-07-02: created
\
/ Author:	Daniel Lohmann (daniel@losoft.de)
\			http://www.losoft.de/
/
\
/ Please send comments and bugfixes to the above email address.
\
/ This code is provided AS IS, you use it at your own risk! 
\ You may use it for whatever you want.
/
\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#ifndef __WIN_ADAPTER_H__
#define __WIN_ADAPTER_H__

#define WIN_ADAPTER_GENERATE_UNIQUE ( __LINE__ + 0x00000000 )

namespace win {

	namespace adapter {
		
		// The following struct is used as base class for all callback adapters
		// It just declares and initializes the adapters member data, so we don't have
		// to do this seperately for each single adapter. 
		template< class O, class CB, class U >
		struct CB_Base
		{
			typedef CB_Base super_t;

			CB_Base( O* pObj, CB pfCB, U UserData )
				: m_pObj( pObj ), m_pfCB( pfCB ), m_UserData( UserData ) {}

			O*	m_pObj;
			CB	m_pfCB;
			U	m_UserData;
		};

		/////////////////////////////////////////////////////////////////////////
		// Callback adapter for callback functions with a user defined argument.
		// This is any callback function following the form:
		//
		//     R CALLBACK MyCallback( [A1 arg1, A2 arg2, A3 arg2, ...], An UserData );
		//
		// where:
		//
		//     R		- return type of the callback fn
		//     A1		- type of the callbacks first argument
		//	   A2, ...  - type of the callbacks second, third, ... argument
		//	   An		- type of the callbacks last argument. This argument contains user defined
		//                data passed to the callback and is usually passed together with the
		//				  callbacks adress to the API that invokes the callback. An has to be
		//				  a 32 bit data type like void*, LONG, DWORD, etc.
		//
		// The adapter redirects the callback to a member fn of class O. 
		// This member fn can be any virtual or non-virtual function of the form:
		//
		//	   [virtual] R O::MyMemberCallback( [A1 arg1, A2 arg2, A3 arg2, ...], U UserData );
		//
		// where:
		//
		//	   R,A1...  - same as above
		//	   U		- type of some user defined data passed to the callback.
		//				  This replaces the An type of the original callback, but can be
		//				  any build in or user defined type
		//
		// The O, R, U, A1...An types are passed as template arguments to the adapter.
		//
		//////////////////
		// 
		// Example: Redirecting EnumWindows() callback to a member fn:
		//
		//	a) Choose an appropriate adapter:
		//     The prototype of the callback fn passed to EnumWindows is
		//         BOOL EnumWindowsProc( HWND hwnd, LPARAM lUserData )
		//     It takes two parameters, so use the CB2 adapter class.
		//  b) Choose the type of the user defined data.
		//     In the example an struct type is used. If you don't need user defined data     
		//	   choose int and pass 0 for the data.
		//
		//  struct userdata_t {
		//	    ...
		//  };
		//  class Test {
		//  public:
		//      BOOL MyEnumWindowsProc( HWND hWnd, userdata_t UserData ) {
		//			...
		//		}
		//		...
		//	};
		//
		//
		//  void foo() {
		//	    userdata_t UserData;
		//   	...
		//		Test t;
		//
		//		// Initialize Adapter
		//      win::adapter::CB2< Test, userdata_t, HWND, LPARAM> theAdapter( &t, &Test::MyEnumWindowsProc, UserData );
		//		// Call EnumWindows. Use adapters Callback fn and pass adapter instance for the
		//		// user defined data
		//      EnumWindows( theAdapter.Callback, (LPARAM) &theAdapter );
		//	}
		///////////////////////////////////////////////////////////////////////////
		
		
		// Adapter for callback functions with two arguments (last argument user defined).
		//     R CALLBACK MyCallback( A1 arg1, A2 Data ) is redirected to
		//     [virtual] R O::MyMemberCallback( A1 arg1, U UserData );
		template< class O, class U, class R, class A1, class A2 >
		struct CB2 : public CB_Base<O, R (O::*)(A1,U), U>
		{
			CB2( O* pObj, R (O::*pfCB)(A1, U), U UserData )
				: super_t( pObj, pfCB , UserData ) {}

			static R CALLBACK Callback( A1 arg1, A2 pObj )
			{
				CB2* _this = (CB2*)pObj;
				return (_this->m_pObj->*_this->m_pfCB)( arg1, _this->m_UserData );
			}
		};

		// Adapter for callback functions with three arguments (last argument user defined).
		//     R CALLBACK MyCallback( A1 arg1, A2 arg2, A3 Data ) is redirected to
		//     [virtual] R O::MyMemberCallback( A1 arg1, A2 arg2, U UserData );
		template< class O, class U, class R, class A1, class A2, class A3 >
		struct CB3 : public CB_Base<O, R (O::*)(A1,A2,U), U>
		{
			CB3( O* pObj, R (O::*pfCB)(A1, A2, U), U UserData )
				: super_t( pObj, pfCB , UserData ) {}

			static R CALLBACK Callback( A1 arg1, A2 arg2, A3 pObj )
			{
				CB3* _this = (CB3*)pObj;
				return (_this->m_pObj->*_this->m_pfCB)( arg1, arg2, _this->m_UserData );
			}
		};

		// Adapter for callback functions with four arguments (last argument user defined).
		//     R CALLBACK MyCallback( A1 arg1, A2 arg2, A3 arg3, A4 Data ) is redirected to
		//     [virtual] R O::MyMemberCallback( A1 arg1, A2 arg2, A3 arg3, U UserData );
		template< class O, class U, class R, class A1, class A2, class A3, class A4>
			struct CB4 : public CB_Base<O, R (O::*)(A1,A2,A3,U), U>
		{
			CB4( O* pObj, R (O::*pfCB)(A1, A2, A3, U), U UserData )
				: super_t( pObj, pfCB , UserData ) {}

			static R CALLBACK Callback( A1 arg1, A2 arg2, A3 arg3, A4 pObj )
			{
				CB4* _this = (CB4*)pObj;
				return (_this->m_pObj->*_this->m_pfCB)( arg1, arg2, arg3, _this->m_UserData );
			}
		};

		// Adapter for callback functions with five arguments (last argument user defined).
		//     R CALLBACK MyCallback( A1 arg1, A2 arg2, A3 arg3, A4 arg4, A5 Data ) is redirected to
		//     [virtual] R O::MyMemberCallback( A1 arg1, A2 arg2, A3 arg3, A4 arg4, U UserData );
		template< class O, class U, class R, class A1, class A2, class A3, class A4, class A5>
			struct CB5 : public CB_Base<O, R (O::*)(A1,A2,A3,A4,U), U>
		{
			CB5( O* pObj, R (O::*pfCB)(A1, A2, A3, A4, U), U UserData )
				: super_t( pObj, pfCB , UserData ) {}

			static R CALLBACK Callback( A1 arg1, A2 arg2, A3 arg3, A4 arg4, A5 pObj )
			{
				CB5* _this = (CB5*)pObj;
				return (_this->m_pObj->*_this->m_pfCB)( arg1, arg2, arg3, arg4, _this->m_UserData );
			}
		};

		// Adapter for callback functions with one user defined argument and adapter instance 
		// created on the heap. The adapter instance is automatically destroyed with delete
		// after processing the callback. This is used for CreateThread() and _beginthreadex
		//     R CALLBACK MyCallback( A1 arg1 ) is redirected to
		//     [virtual] R O::MyMemberCallback( U UserData );
		template< class O, class U, class R, class A1 >
		struct HeapCB1 : public CB_Base<O, R (O::*)(U), U>
		{
			static HeapCB1* Create( O* pObj, R (O::*pfCB)(U), U UserData )
			{
				return new HeapCB1( pObj, pfCB, UserData );
			}

			static R CALLBACK CallbackAndDestroy( A1  pObj )
			{
				HeapCB1* _this = (HeapCB1*)pObj;
				R Result = (_this->m_pObj->*_this->m_pfCB)(_this->m_UserData );
				delete _this;
				return Result;
			}
		protected:
			// Constructor is protected to prevent instances on the stack
			HeapCB1( O* pObj, R (O::*pfCB)(U), U UserData )
				: super_t( pObj, pfCB , UserData ) {}
		};

	
		/////////////////////////////////////////////////////////////////////////
		// Callback adapter for callback functions *without* a user defined argument.
		// This is any callback function following the form:
		//
		//     R CALLBACK MyCallback( [A1 arg1, A2 arg2, A3 arg2, ...] );
		//
		// The adapter redirects the callback to a member fn of class O. 
		// This member fn can be any virtual or non-virtual function of the form:
		//
		//	   [virtual] R O::MyMemberCallback( [A1 arg1, A2 arg2, A3 arg2, ...], U UserData );
		//
		//
		// Because there is no user defined data passed to the callbacks, it is not possible
		// to pass the adapters instance as an argument. Instead it is stored in a static 
		// data member. To ensure that the code also works in multi threaded environments, 
		// thread local storage is used for the static data member.
		// To ensure that not two different adapters, that are accidentally of the same type,
		// share the same class with the same instance of the static data, an additional int
		// template argument called INSTANCE is passed to the adapters. You have to ensure
		// that every instanciation of the template uses a different value for INSTANCE.
		//
		/////////////////////////////////////////////////////////////////////////

		template< int INSTANCE, class O, class U, class R, class A1 >
		struct StaticCB1 : public CB_Base<O, R (O::*)(A1,U), U> 
		{
			StaticCB1( O* pObj, R (O::*pfCB)(A1, U), U UserData )
				: super_t( pObj, pfCB , UserData )
			{
				StaticCB1::_this = this;
			}

			static R CALLBACK Callback( A1 arg1 )
			{
				return (_this->m_pObj->*_this->m_pfCB)(arg1, _this->m_UserData );
			}

			__declspec( thread ) static StaticCB1* _this;
		};
		template< int INSTANCE, class O, class U, class R, class A1 >
		StaticCB1<INSTANCE, O, U, R, A1>* StaticCB1<INSTANCE, O, U, R, A1>::_this = NULL;

		template< int INSTANCE, class O, class U, class R, class A1, class A2 >
		struct StaticCB2 : public CB_Base<O, R (O::*)(A1,A2,U), U> 
		{
			StaticCB2( O* pObj, R (O::*pfCB)(A1,A2,U), U UserData )
				: super_t( pObj, pfCB , UserData )
			{
				StaticCB2::_this = this;
			}

			static R CALLBACK Callback( A1 arg1, A2 arg2 )
			{
				return (_this->m_pObj->*_this->m_pfCB)(arg1, arg2, _this->m_UserData );
			}

			__declspec( thread ) static StaticCB1* _this;
		};
		template< int INSTANCE, class O, class U, class R, class A1, class A2 >
		StaticCB2<INSTANCE, O, U, R, A1, A2>* StaticCB2<INSTANCE, O, U, R, A1, A2>::_this = NULL;

	}	// namespace adapter


	///////////////////////////////////////////////////////////////////////////
	// Wrappers for APIs that use a callback with user defined 32 Bit data
	//
	
	template< class O, class U >
	BOOL EnumWindows( O* obj, BOOL (O::*pfMemFunc)(HWND, U), U UserData = U() )
	{
		typedef adapter::CB2<O, U, BOOL, HWND, LPARAM> adapter_t;
		return ::EnumWindows( adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	BOOL EnumChildWindows( HWND hWnd, O* obj, BOOL (O::*pfMemFunc)(HWND, U), U UserData = U() )
	{
		typedef adapter::CB2<O, U, BOOL, HWND, LPARAM> adapter_t;
		return ::EnumChildWindows( hWnd, adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	BOOL EnumDesktopWindows( HDESK hDesk, O* obj, BOOL (O::*pfMemFunc)(HWND, U), U UserData = U() )
	{
		typedef adapter::CB2<O, U, BOOL, HWND, LPARAM> adapter_t;
		return ::EnumDesktopWindows( hDesk, adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	BOOL EnumDesktops( HWINSTA hWinSta, O* obj, BOOL (O::*pfMemFunc)(LPTSTR, U), U UserData = U() )
	{
		typedef adapter::CB2<O, U, BOOL, LPTSTR, LPARAM> adapter_t;
		return ::EnumDesktops( hWinSta, adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ) );
	}
	
	template< class O, class U >
	BOOL EnumWindowStations( O* obj, BOOL (O::*pfMemFunc)(LPTSTR, U), U UserData = U() )
	{
		typedef adapter::CB2<O, U, BOOL, LPTSTR, LPARAM> adapter_t;
		return ::EnumWindowStations( adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	BOOL EnumResourceTypes( HMODULE hModule, O* obj, BOOL (O::*pfMemFunc)(HMODULE, LPTSTR, U), U UserData = U() )
	{
		typedef adapter::CB3<O, U, BOOL, HMODULE, LPTSTR, LONG_PTR> adapter_t;
		return ::EnumResourceTypes( hModule, adapter_t::Callback, (LONG_PTR) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	BOOL EnumResourceNames( HMODULE hModule, LPCTSTR pszType, O* obj, BOOL (O::*pfMemFunc)(HMODULE, LPCTSTR, LPTSTR, U), U UserData = U() )
	{
		typedef adapter::CB4<O, U, BOOL, HMODULE, LPCTSTR, LPTSTR, LONG_PTR> adapter_t;
		return ::EnumResourceNames( hModule, pszType, adapter_t::Callback, (LONG_PTR) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	BOOL EnumResourceLanguages( HMODULE hModule, LPCTSTR pszType, LPCTSTR pszName, O* obj, BOOL (O::*pfMemFunc)(HMODULE, LPCTSTR, LPCTSTR, WORD, U), U UserData = U() )
	{
		typedef adapter::CB5<O, U, BOOL, HMODULE, LPCTSTR, LPCTSTR, WORD, LONG_PTR> adapter_t;
		return ::EnumResourceLanguages( hModule, pszType, pszName, adapter_t::Callback, (LONG_PTR) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	BOOL EnumThreadWindows( DWORD dwThreadId, O* obj, BOOL (O::*pfMemFunc)(HWND, U), U UserData = U() )
	{
		typedef adapter::CB2<O, U, BOOL, HWND, LPARAM> adapter_t;
		return ::EnumThreadWindows( dwThreadId, adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	int EnumFonts( HDC hDC, LPCTSTR pszFaceName, O* obj, int (O::*pfMemFunc)(CONST LOGFONT*, CONST TEXTMETRIC*, DWORD, U), U UserData = U() )
	{
		typedef adapter::CB4<O, U, int, CONST LOGFONT*, CONST TEXTMETRIC*, DWORD, LPARAM> adapter_t;
		return ::EnumFonts( hDC, pszFaceName, adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	int EnumFontFamiliesEx( HDC hDC, LPLOGFONT pLogFont, O* obj, int (O::*pfMemFunc)(ENUMLOGFONTEX*, NEWTEXTMETRICEX*, DWORD, U), U UserData = U(), DWORD dwFlags = 0 )
	{
		typedef adapter::CB4<O, U, int, ENUMLOGFONTEX*, NEWTEXTMETRICEX*, DWORD, LPARAM> adapter_t;
		return ::EnumFontFamiliesEx( hDC, pLogFont, (FONTENUMPROC) adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ), dwFlags );
	}

	template< class O, class U >
	BOOL EnumMetaFile( HDC hDC, HMETAFILE hmf, O* obj, int (O::*pfMemFunc)(HDC, HANDLETABLE*, METARECORD*, int, U), U UserData = U() )
	{
		typedef adapter::CB5<O, U, int, HDC, HANDLETABLE*, METARECORD*, int, LPARAM> adapter_t;
		return ::EnumMetaFile( hDC, hmf, adapter_t::Callback, (LONG) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >	// Note: Order of parameters is slightly changed
	BOOL EnumEnhMetaFile( HDC hDC, HENHMETAFILE hemf, LPCRECT prcClip, O* obj, int (O::*pfMemFunc)(HDC, HANDLETABLE*, CONST ENHMETARECORD*, int, U), U UserData = U() )
	{
		typedef adapter::CB5<O, U, int, HDC, HANDLETABLE*, CONST ENHMETARECORD*, int, LPARAM> adapter_t;
		return ::EnumEnhMetaFile( hDC, hemf, adapter_t::Callback, (LPVOID) &adapter_t( obj, pfMemFunc, UserData ), prcClip );
	}

	template< class O, class U >
	int EnumICMProfiles( HDC hDC, O* obj, int (O::*pfMemFunc)(LPTSTR, U), U UserData = U() )
	{
		typedef adapter::CB2<O, U, int, LPTSTR, LPARAM> adapter_t;
		return ::EnumICMProfiles( hDC, adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	int EnumObjects( HDC hDC, int nType, O* obj, int (O::*pfMemFunc)(LPVOID, U), U UserData = U() )
	{
		typedef adapter::CB2<O, U, int, LPVOID, LPARAM> adapter_t;
		return ::EnumObjects( hDC, nType, adapter_t::Callback, (LPARAM) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	BOOL EnumerateLoadedModules( HANDLE hProcess, O* obj, BOOL (O::*pfMemFunc)(PSTR, ULONG, ULONG, U), U UserData = U() )
	{
		typedef adapter::CB4< O, U, BOOL, PSTR, ULONG, ULONG, PVOID> adapter_t;
		return ::EnumerateLoadedModules( hProcess, adapter_t::Callback, (PVOID) &adapter_t( obj, pfMemFunc, UserData ) );
	}

	template< class O, class U >
	BOOL EnumDisplayMonitors( HDC hDC, LPCRECT prcClip, O* obj, BOOL (O::*pfMemFunc)(HMONITOR, HDC, LPRECT, U), U UserData = U() )
	{
		typedef adapter::CB4<O, U, BOOL, HMONITOR, HDC, LPRECT, LPARAM> adapter_t;
		return ::EnumDisplayMonitors( hDC, prcClip, adapter_t::Callback, (DWORD) &adapter_t( obj, pfMemFunc, UserData ) );
	}


	///////////////////////////////////////////////////////////////////////////
	// Wrappers for APIs that use a callback that is called 
	// asynchronously from the current thread.
	//

	template< class O, class U > // Note: Order of parameters is changed
	HANDLE CreateThread( O* obj, DWORD (O::*pfThreadStart)(U), U UserData = U(), PDWORD pThreadId = NULL, DWORD dwCreationFlags = 0,  DWORD dwStackSize = 0, PSECURITY_ATTRIBUTES pSA = NULL )
	{
		typedef adapter::HeapCB1<O, U, DWORD, LPVOID> adapter_t;
		DWORD dwDummy;
		return ::CreateThread( pSA, dwStackSize, adapter_t::CallbackAndDestroy, 
							   (LPVOID) adapter_t::Create( obj, pfThreadStart, UserData ),
							   dwCreationFlags, pThreadId ? pThreadId : &dwDummy
				);
	}

	template< class O, class U > // Note: Order of parameters is changed
	HANDLE beginthreadex( O* obj, DWORD (O::*pfThreadStart)(U), U UserData = U(), PDWORD pThreadId = NULL, DWORD dwCreationFlags = 0,  DWORD dwStackSize = 0, PSECURITY_ATTRIBUTES pSA = NULL )
	{
		typedef adapter::HeapCB1<O, U, DWORD, LPVOID> adapter_t;
		DWORD dwDummy;
		return (HANDLE)::_beginthreadex( pSA, dwStackSize, (unsigned int (__stdcall *)(void *))adapter_t::CallbackAndDestroy, 
									     (LPVOID) adapter_t::Create( obj, pfThreadStart, UserData ),
									     dwCreationFlags, PUINT( pThreadId ? pThreadId : &dwDummy )
				);
	}


	///////////////////////////////////////////////////////////////////////////
	// Wrappers for APIs that use a callback without 
	// a user defined parameter. 
	//

	template< class O, class U >
	BOOL EnumSystemCodePages( O* obj, BOOL (O::*pfMemFunc)(LPTSTR, U), DWORD dwFlags, U UserData )
	{
		typedef adapter::StaticCB1< WIN_ADAPTER_GENERATE_UNIQUE, O, U, BOOL, LPTSTR > adapter_t;
		adapter_t theAdapter( obj, pfMemFunc, UserData );
		return ::EnumSystemCodePages( theAdapter.Callback, dwFlags );
	}

	template< class O, class U >
	BOOL EnumSystemLocales( O* obj, BOOL (O::*pfMemFunc)(LPTSTR, U), DWORD dwFlags, U UserData )
	{
		typedef adapter::StaticCB1< WIN_ADAPTER_GENERATE_UNIQUE, O, U, BOOL, LPTSTR > adapter_t;
		adapter_t theAdapter( obj, pfMemFunc, UserData );
		return ::EnumSystemLocales( theAdapter.Callback, dwFlags );
	}

	template< class O, class U >
	BOOL EnumDateFormats( O* obj, BOOL (O::*pfMemFunc)(LPTSTR, U), LCID Locale, DWORD dwFlags, U UserData = 0 )
	{
		typedef adapter::StaticCB1< WIN_ADAPTER_GENERATE_UNIQUE, O, U, BOOL, LPTSTR > adapter_t;
		adapter_t theAdapter( obj, pfMemFunc, UserData );
		return ::EnumDateFormats( theAdapter.Callback, Locale, dwFlags );
	}

	template< class O, class U >
	BOOL EnumDateFormatsEx( O* obj, BOOL (O::*pfMemFunc)(LPTSTR, U), LCID Locale, DWORD dwFlags, U UserData = 0 )
	{
		typedef adapter::StaticCB2< WIN_ADAPTER_GENERATE_UNIQUE, O, U, BOOL, LPTSTR, CALID > adapter_t;
		adapter_t theAdapter( obj, pfMemFunc, UserData );
		return ::EnumDateFormatsEx( theAdapter.Callback, Locale, dwFlags );
	}

	template< class O, class U >
	BOOL EnumTimeFormats( O* obj, BOOL (O::*pfMemFunc)(LPTSTR, U), LCID Locale, DWORD dwFlags, U UserData = 0 )
	{
		typedef adapter::StaticCB1< WIN_ADAPTER_GENERATE_UNIQUE, O, U, BOOL, LPTSTR > adapter_t;
		adapter_t theAdapter( obj, pfMemFunc, UserData );
		return ::EnumTimeFormats( theAdapter.Callback, Locale, dwFlags );
	}

	template< class O, class U >
	BOOL EnumCalendarInfo( O* obj, BOOL (O::*pfMemFunc)(LPTSTR, U), LCID Locale, CALID Calendar, CALTYPE CalType, U UserData = 0 )
	{
		typedef adapter::StaticCB1< WIN_ADAPTER_GENERATE_UNIQUE, O, U, BOOL, LPTSTR > adapter_t;
		adapter_t theAdapter( obj, pfMemFunc, UserData );
		return ::EnumCalendarInfo( theAdapter.Callback, Locale, Calendar, CalType );
	}


}	// namespace win


#endif // __WIN_ADAPTER_H__
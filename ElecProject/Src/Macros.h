#pragma once

#define DEFAULT_EXCEPT() (BaseException(__LINE__,__FILE__))
#define WINDOW_EXCEPT(hr) (Window::Exception(__LINE__,__FILE__,hr))
#define WINDOW_LAST_ERROR_EXCEPT() (WINDOW_EXCEPT(GetLastError()))
#define GFX_EXCEPT(hr) (Graphics::Exception(__LINE__,__FILE__,hr))


#define THROW_FAILED_GFX(f) if( FAILED(hr = f) ) throw Graphics::Exception(__LINE__,__FILE__,hr)


// Todo add macro and possible exception that actually tells us file that was failed to find
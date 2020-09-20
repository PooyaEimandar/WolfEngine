#include "w_render_pch.h"
#include "w_game.h"
#include <future>

using namespace std;
using namespace wolf;
//using namespace wolf::graphics;
using namespace wolf::framework;

w_game::w_game(_In_z_ const std::wstring& pContentPath, _In_ const system::w_logger_config& pLogConfig) :
	exiting(false),
	_loaded(false),
	_is_released(false),
    _app_name(pLogConfig.app_name)
{
	content_path = pContentPath;

    if (!logger.get_is_open())
    {
#ifdef __UWP
        logger.initialize(pAppName);
#else
        logger.initialize(pLogConfig);
#endif
    }
}

w_game::~w_game()
{
}

void w_game::initialize(_In_ map<int, w_present_info> pOutputWindowsInfo)
{	
	w_graphics_device_manager::initialize(pOutputWindowsInfo);
}

void w_game::load()
{
}

void w_game::update(_In_ const wolf::system::w_game_time& pGameTime)
{
	W_UNUSED(pGameTime);
}

W_RESULT w_game::render(_In_ const wolf::system::w_game_time& pGameTime)
{
#ifdef	__DX11__
    
#pragma region show printf on screen log messages
    
    auto _msgs = logger.get_buffer();
    auto _size = _msgs.size();
    if (_size != 0)
    {
        this->sprite_batch->begin();
        {
            std::wstring _final_msg = L"";
            for (size_t i = 0; i < _size; ++i)
            {
                _final_msg += _msgs.at(i) + L"\r\n";
            }
            this->sprite_batch->draw_string(_final_msg,
                                            D2D1::RectF(5, 5, static_cast<FLOAT>(get_window_width() / 2), static_cast<FLOAT>(get_window_height())));
        }
        this->sprite_batch->end();
        
        logger.clearf();
    }
    
#pragma endregion
    
#endif
            
    return w_graphics_device_manager::present();
}

bool w_game::run(_In_ map<int, w_present_info>& pOutputWindowsInfo)
{
	if (!this->_loaded)
	{
		//Async initialize & load
		auto f = std::async(std::launch::async, [this, pOutputWindowsInfo]()->void
		{
			initialize(pOutputWindowsInfo);
			load();
			this->_loaded = true;
		});
		std::chrono::milliseconds _milli_sec(16);
		f.wait_for(_milli_sec);
	}

    if (!this->_loaded) return true;

    update(this->_game_time);

    this->_game_time.tick([&]()
    {
        if (w_graphics_device_manager::prepare() == W_PASSED)
        {
            render(this->_game_time);
        }
    });
	
	//reset keyboard buffers for next cycle
	inputs_manager.reset_keyboard_buffers();

    return !this->exiting;
}

void w_game::exit()
{
    //std::exit(pExitCode);
    this->exiting = true;
}

ULONG w_game::release()
{
    if (this->_is_released) return 1;
	
    this->exiting = true;
	this->_is_released = true;

	return 0;
}

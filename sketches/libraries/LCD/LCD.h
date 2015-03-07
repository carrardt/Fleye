#ifndef _Template_LiquidCrystal_h
#define _Template_LiquidCrystal_h

#include <inttypes.h>
#include <Wiring.h>

template<uint8_t p0, uint8_t... p>
struct PinSetHelper;

template<uint8_t p0>
struct PinSetHelper<p0>
{
	static void setmode(int m)
	{
		pinMode(p0,m);
	}
	static void writeBits(unsigned int x) 
	{
		digitalWrite( p0, x & 0x01 );
	}
};

template<uint8_t p0, uint8_t... p>
struct PinSetHelper
{
	static void setmode(int m)
	{
		pinMode(p0,m);
		PinSetHelper<p...>::setmode(m);
	}
	static void writeBits(unsigned int x) 
	{
		digitalWrite( p0, x & 0x01 );
		PinSetHelper<p...>::writeBits( x >> 1);
	}
};

template<uint8_t... p>
class PinSet
{
public:
	static constexpr unsigned int count = sizeof...(p);
	static void writeBits(unsigned int x) { PinSetHelper<p...>::writeBits(x); }
	static void setmode(int m) { PinSetHelper<p...>::setmode(m); }
};

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

// handles Read/Write mode with optional rw pin
// if no pin is connected (pin = -1) nothing happens
template<int _rw=-1>
class LCDRW
{
public:
	static constexpr uint8_t rw_pin = _rw;
	static void init() { pinMode(rw_pin,OUTPUT); }
	static void setWritable() { digitalWrite(rw_pin,LOW); }
	static void setReadable() { digitalWrite(rw_pin,HIGH); }
};
template<>
class LCDRW<-1>
{
public:
	static void init() { }
	static void setWritable() { }
	static void setReadable() { }
};

// handles low level data transmission
template<uint8_t _rs, uint8_t _en, class _pins>
class LCDDataPinsBase
{
public:
	static constexpr uint8_t rs_pin = _rs; 
	static constexpr uint8_t en_pin = _en;
	using pins = _pins;
	static constexpr unsigned int pinCount = pins::count;
	
	static void setWritable()
	{
		pins::setmode(OUTPUT);
	}
	
	static void setReadable()
	{
		pins::setmode(INPUT);
	}
	
	static void initCommon()
	{
		pinMode(rs_pin,OUTPUT);
		pinMode(en_pin,OUTPUT);
		digitalWrite(rs_pin,LOW);
		digitalWrite(en_pin,LOW);
		setWritable();
	}
	
	static void pulseEnable()
	{
  	  digitalWrite(en_pin, LOW);
  	  delayMicroseconds(1);    
  	  digitalWrite(en_pin, HIGH);
  	  delayMicroseconds(1);    // enable pulse must be >450ns
  	  digitalWrite(en_pin, LOW);
  	  delayMicroseconds(100);   // commands need > 37us to settle
	}
}; 

// 4/8 bits mode template
template<uint8_t _rs, uint8_t _en, class _pins, unsigned int npins=_pins::count>
class LCDDataPins; 

// 8-bits specialization
template<uint8_t _rs, uint8_t _en, class _pins>
class LCDDataPins<_rs,_en,_pins,8> : public LCDDataPinsBase<_rs,_en,_pins>
{
public:
	using Super = LCDDataPinsBase<_rs,_en,_pins>;
	using data_pins = typename Super::pins; 
	static constexpr uint8_t data_mode = LCD_8BITMODE;

	static void init(uint8_t displayFunction)
	{
		Super::initCommon();
		write(LCD_FUNCTIONSET | displayFunction);
    		delayMicroseconds(4500);  // wait more than 4.1ms
		write(LCD_FUNCTIONSET | displayFunction);
    		delayMicroseconds(150);
		write(LCD_FUNCTIONSET | displayFunction);
	}

	static void write(uint8_t value)
	{
		data_pins::writeBits( value );
		Super::pulseEnable();
	}
};

// 4-bits specialization
template<uint8_t _rs, uint8_t _en, class _pins>
class LCDDataPins<_rs,_en,_pins,4> : public LCDDataPinsBase<_rs,_en,_pins>
{
public:
	using Super = LCDDataPinsBase<_rs,_en,_pins>;
	using data_pins = typename Super::pins; 
	static constexpr uint8_t data_mode = LCD_4BITMODE;

	static void init(uint8_t displayMode)
	{
		Super::initCommon();
		write4bits(0x03);
		delayMicroseconds(4500);
		write4bits(0x03);
		delayMicroseconds(4500);
		write4bits(0x03);
		delayMicroseconds(150);
		write4bits(0x02);
	}
	static void write4bits(uint8_t value)
	{
		data_pins::writeBits( value );
		Super::pulseEnable();
	}
	static void write(uint8_t value)
	{
		write4bits(value>>4);
		write4bits(value);
	}
};

template<class _RWPin, class _DataPins, int _cols, int _lines, uint8_t _dotSize>
class LCDBase
{
	using RWPin = _RWPin;
	using DataPins = _DataPins;
	static constexpr int cols = _cols;
	static constexpr int lines = _lines;
	static constexpr uint8_t lineMode = (_lines<=1) ? LCD_1LINE : LCD_2LINE ;
	static constexpr uint8_t dotSize = _dotSize;
	static constexpr uint8_t displayFlags = DataPins::data_mode | lineMode | dotSize;	
public:
	// TODO: m_WriteMode should be in RWPin class and stored if necessary only
	LCDBase()
		: m_writeMode(true)
		, m_displayControlFlags(0)
		, m_displayModeFlags(0)
	{
	}

	void begin()
	{
		init( displayFlags );
		command( LCD_FUNCTIONSET | displayFlags );
		m_writeMode = true;
		setDisplayFlags( LCD_DISPLAYON );
		clear();
  		setDisplayModeFlags( LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
	}

	void print(const char* txt)
	{
		if( txt == 0) { print("<null>"); return; }
		for(int i=0;txt[i]!='\0';++i)
			write(txt[i]);
	}

	void print(const char c)
	{
		write( c );
	}
	
	void print(unsigned long x) { print((long)x); }
	void print(unsigned int x) { print((long)x); }
	void print(int x) { print((long)x); }
	void print(long x)
	{
		char digits[12];
		int n = 0;
		if( x < 0 ) { write('-'); x=-x; }
		do
		{
			digits[n++] = x % 10;
			x /= 10;
		} while( x > 0 );
		for(int i=0;i<n;i++) { write('0'+digits[n-i-1]); }
	}

	template<class T>
	inline LCDBase& operator << (const T& x)
	{
		print(x);
		return *this;
	}

	static void clear()
	{
	  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
	  delayMicroseconds(2000);  // this command takes a long time!
	}
	
	static void home()
	{
	  command(LCD_RETURNHOME);  // set cursor position to zero
	  delayMicroseconds(2000);  // this command takes a long time!
	}
	
	static void setCursor(uint8_t col, uint8_t row)
	{
	  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	  if ( row >= lines ) {
	    row = lines-1;    // we count rows starting w/0
	  }
 	 command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
	}

	void setDisplayFlags(uint8_t flags)
	{
		if( (m_displayControlFlags&flags) == flags ) return;
		m_displayControlFlags |= flags;
	  	command(LCD_DISPLAYCONTROL | m_displayControlFlags);
	}
	void clearDisplayFlags(uint8_t flags)
	{
		if( (m_displayControlFlags&flags) == 0 ) return;
		m_displayControlFlags &= ~flags;
	  	command(LCD_DISPLAYCONTROL | m_displayControlFlags);
	}

	void noDisplay() { clearDisplayFlags(LCD_DISPLAYON); }
	void display() { setDisplayFlags(LCD_DISPLAYON); }
	void noCursor() { clearDisplayFlags(LCD_CURSORON); }
	void cursor() { setDisplayFlags(LCD_CURSORON); }
	void noBlink() { clearDisplayFlags(LCD_BLINKON); }
	void blink() { setDisplayFlags(LCD_BLINKON); }

	void scrollDisplayLeft()
	{
	  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
	}
	void scrollDisplayRight()
	{
	  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
	}
	
	void setDisplayModeFlags(uint8_t flags)
	{
	  if( (m_displayModeFlags&flags) == flags ) return;
	  m_displayModeFlags |= flags;
	  command(LCD_ENTRYMODESET | m_displayModeFlags);
	}
	void clearDisplayModeFlags(uint8_t flags)
	{
	  if( (m_displayModeFlags&flags) == 0 ) return;
	  m_displayModeFlags &= ~flags;
	  command(LCD_ENTRYMODESET | m_displayModeFlags);
	}
	
	void leftToRight() { setDisplayModeFlags(LCD_ENTRYLEFT); }
	void rightToLeft() { clearDisplayModeFlags(LCD_ENTRYLEFT); }
	void autoscroll() { setDisplayModeFlags(LCD_ENTRYSHIFTINCREMENT); }
	void noAutoscroll() { clearDisplayModeFlags(LCD_ENTRYSHIFTINCREMENT); }

	static void createChar(uint8_t location, uint8_t charmap[])
	{
	  location &= 0x7; // we only have 8 locations 0-7
	  command(LCD_SETCGRAMADDR | (location << 3));
	  for (int i=0; i<8; i++) {
	    write(charmap[i]);
	  }
	}

	static void command(uint8_t value)
	{
		digitalWrite(DataPins::rs_pin,LOW);
		DataPins::write(value);
	}

	static void write(uint8_t value)
	{
		digitalWrite(DataPins::rs_pin,HIGH);
		DataPins::write(value);
	}

private:
	bool m_writeMode;
	uint8_t m_displayControlFlags;
	uint8_t m_displayModeFlags;

	static void init(uint8_t displayFlags)
	{
		RWPin::init();
		DataPins::init(displayFlags);
	}
	
	static void setWritable()
	{
		RWPin::setWritable();
		DataPins::setWritable();
	}
};

template < uint8_t rs, uint8_t en, class pins, int c=16, int l=2, uint8_t ds=LCD_5x8DOTS, int rw=-1>
using LCD =
LCDBase< LCDRW<rw>, LCDDataPins<rs,en,pins> ,c,l,ds >;

#endif

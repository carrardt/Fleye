#ifndef __fleye_plugin_TextService_h
#define __fleye_plugin_TextService_h

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "fleye/service.h"

struct StringStreamHelper : public std::ostringstream
{
	inline StringStreamHelper() : m_string(0) {}
	inline StringStreamHelper( const StringStreamHelper& ssh ) : m_string(ssh.m_string) {}
	inline StringStreamHelper( StringStreamHelper&& ssh ) : m_string(std::move(ssh.m_string)) { ssh.m_string=0; }
	inline StringStreamHelper( std::string* s ) : m_string(s) {}
	inline ~StringStreamHelper() { if(m_string!=0) *m_string = str(); }
	std::string * m_string;
};

struct PositionnedText
{
	static constexpr int MaxSize = 1024;
	
	float x, y;
	std::string text;
	
	inline PositionnedText() : x(0.0f), y(0.0f) { text[0]='\0'; }
	
	inline StringStreamHelper out() { return StringStreamHelper(&text); }
	
	inline void setText(std::string s)
	{
		text = s;
	}
	inline std::string getText() const { return text; }
};

class TextService : public FleyeService
{
  public:
	inline PositionnedText* addPositionnedText(float x, float y)
	{
		PositionnedText* t = new PositionnedText;
		t->x=x;
		t->y=y;
		m_posTexts.push_back(t);
		return t;
	}

	inline std::ostringstream& console() { return m_console; }
	
	// text screen size
	inline int lines() const {return 25;}
	inline int columns() const {return 60;}
	
	// console lines
	inline int consoleLines() const {return 4;}
	
	inline std::string consoleText()
	{
		std::string s = m_console.str();
		int startPos = 0;
		int nLines = 0;
		int len = s.size();
		for(int i=len-1;i>=0 && startPos==0;--i)
		{
			if( s[i]=='\n' ) ++nLines;
			if( nLines > consoleLines() ){  startPos=i+1; }
		}
		if( startPos > 0 )
		{
			//std::cout<<"clear, startPos="<<startPos<<"\n";
			m_console.str("");
			m_console << s.substr(startPos);
		}
		return m_console.str();
	}

	const std::vector<PositionnedText*>& getPositionnedTexts() const { return m_posTexts; };
	
  private:
	std::vector<PositionnedText*> m_posTexts;
	std::ostringstream m_console;
};

FLEYE_DECLARE_SERVICE(TextService)

#endif

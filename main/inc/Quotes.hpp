#pragma once

class Quotes
{
public:
	static const char* getStarWarsQuote();
	static const char* getLOTRQuote();
	static int getStarWarsQuoteCount();
	static int getLOTRQuoteCount();

private:
	static const char* star_wars_quotes[];
	static const char* lotr_quotes[];
	static const int star_wars_count;
	static const int lotr_count;
};

#include "Quotes.hpp"
#include "esp_random.h"

const char* Quotes::star_wars_quotes[] = {
	"May the Force be with you",
	"I have a bad feeling about this",
	"Do or do not, there is no try",
	"The Force will be with you, always",
	"I find your lack of faith disturbing",
	"These aren't the droids you're looking for",
	"It's a trap!",
	"I am your father",
	"Help me Obi-Wan Kenobi, you're my only hope",
	"Never tell me the odds",
	"The Force is strong with this one",
	"Fear is the path to the dark side",
	"In my experience there is no such thing as luck",
	"You were the chosen one!",
	"I've got a bad feeling about this",
	"So this is how liberty dies, with thunderous applause",
	"The ability to speak does not make you intelligent",
	"Your focus determines your reality",
	"Wars not make one great",
	"Size matters not"
};

const char* Quotes::lotr_quotes[] = {
	"One does not simply walk into Mordor",
	"You shall not pass!",
	"My precious",
	"Even the smallest person can change the course of the future",
	"Not all those who wander are lost",
	"All we have to decide is what to do with the time that is given us",
	"I wish the Ring had never come to me",
	"There is some good in this world, and it's worth fighting for",
	"The road goes ever on and on",
	"A wizard is never late, nor is he early",
	"I would rather share one lifetime with you than face all the ages alone",
	"Even darkness must pass",
	"Courage is found in unlikely places",
	"Home is behind, the world ahead",
	"Certainty of death, small chance of success, what are we waiting for?",
	"It's the job that's never started as takes longest to finish",
	"Many that live deserve death. And some that die deserve life",
	"The burned hand teaches best",
	"Deeds will not be less valiant because they are unpraised",
	"Moonlight drowns out all but the brightest stars"
};

const int Quotes::star_wars_count = sizeof(star_wars_quotes) / sizeof(star_wars_quotes[0]);
const int Quotes::lotr_count = sizeof(lotr_quotes) / sizeof(lotr_quotes[0]);

const char* Quotes::getStarWarsQuote()
{
	uint32_t rand = esp_random();
	int index = rand % star_wars_count;
	return star_wars_quotes[index];
}

const char* Quotes::getLOTRQuote()
{
	uint32_t rand = esp_random();
	int index = rand % lotr_count;
	return lotr_quotes[index];
}

int Quotes::getStarWarsQuoteCount()
{
	return star_wars_count;
}

int Quotes::getLOTRQuoteCount()
{
	return lotr_count;
}

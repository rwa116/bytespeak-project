// Translator Module

#ifndef _LANGUAGE_MANAGER_HPP_
#define _LANGUAGE_MANAGER_HPP_

#include <string>
#include <unordered_map>

enum Language {
	ENGLISH,
	FRENCH,
	GERMAN,
};

class LanguageManager {
public:
    LanguageManager();
    ~LanguageManager();

	// Get the filename for a specific language
	std::string getWavFilename(enum Language language);
private:
	std::unordered_map<int, std::string> audioFilenames;
};

#endif
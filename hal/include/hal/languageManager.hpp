// Translator Module

#ifndef _LANGUAGE_MANAGER_HPP_
#define _LANGUAGE_MANAGER_HPP_

#include <string>
#include <unordered_map>
#include <vector>

enum Language {
	ENGLISH,
	FRENCH,
	GERMAN,
	SPANISH,
	CHINESE,
	CUSTOM_1,
	CUSTOM_2,
};

class LanguageManager {
public:
    LanguageManager();
    ~LanguageManager();

	// Get the filename for a specific language
	std::string getWavFilename(enum Language language);
	std::vector<enum Language> getDefaultLanguages(void);
private:
	std::unordered_map<int, std::string> audioFilenames;
	std::vector<enum Language> defaultLanguages = {ENGLISH, FRENCH, GERMAN, SPANISH, CHINESE};
};

#endif
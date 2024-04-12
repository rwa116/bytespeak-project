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
	UNKNOWN_LANG,
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
	void cycleLanguage(void);
	enum Language getCurrentLanguage(void);
	void setLanguage(enum Language newLanguage);
private:
	std::unordered_map<int, std::string> audioFilenames;
	std::vector<enum Language> defaultLanguages = {ENGLISH, FRENCH, GERMAN, SPANISH, CHINESE};
	std::vector<enum Language> loadedLanguages = {ENGLISH, FRENCH, GERMAN, SPANISH, CHINESE, CUSTOM_1, CUSTOM_2};
	int currentLanguageIndex;
};

#endif
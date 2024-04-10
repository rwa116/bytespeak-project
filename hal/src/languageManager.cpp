#include <iostream>

#include "hal/languageManager.hpp"

LanguageManager::LanguageManager() {
	//Insert language filenames
	audioFilenames.insert({ENGLISH, "/mnt/remote/myApps/bytespeak-wav-files/english_msg.wav"});
	audioFilenames.insert({FRENCH, "/mnt/remote/myApps/bytespeak-wav-files/french_msg.wav"});
	audioFilenames.insert({GERMAN, "/mnt/remote/myApps/bytespeak-wav-files/german_msg.wav"});
	audioFilenames.insert({SPANISH, "/mnt/remote/myApps/bytespeak-wav-files/spanish_msg.wav"});
	audioFilenames.insert({CHINESE, "/mnt/remote/myApps/bytespeak-wav-files/chinese_msg.wav"});
	audioFilenames.insert({CUSTOM_1, "/mnt/remote/myApps/bytespeak-wav-files/custom1_msg.wav"});
	audioFilenames.insert({CUSTOM_2, "/mnt/remote/myApps/bytespeak-wav-files/custom2_msg.wav"});
}

LanguageManager::~LanguageManager() {

}

std::string LanguageManager::getWavFilename(enum Language language) {
	auto it = audioFilenames.find(language);
	if(it != audioFilenames.end()) {
		return it->second;
	}
	std::cerr << "Could not file wave file for language " << language << std::endl;
	return "";
}

std::vector<enum Language> LanguageManager::getDefaultLanguages(void) {
	return defaultLanguages;
}
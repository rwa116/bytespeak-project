#include <iostream>

#include "hal/languageManager.hpp"

LanguageManager::LanguageManager() {
	//Insert language filenames
	audioFilenames.insert({ENGLISH, "beatbox-wav-files/english_msg.wav"});
	audioFilenames.insert({FRENCH, "beatbox-wav-files/french_msg.wav"});
	audioFilenames.insert({GERMAN, "beatbox-wav-files/german_msg.wav"});
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
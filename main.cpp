#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <regex>

/// @brief Pure function to count occurences of words in a word list
/// @param words The list of words to count
/// @return A map of words to their counts
auto countOccurences(const std::vector<std::string>& words) {
    auto map = [](const std::string& word) {
        return std::make_pair(word, 1);
    };

    std::vector<std::pair<std::string, int>> pairs;
    std::transform(words.begin(), words.end(), std::back_inserter(pairs), map);

    auto reduce = [](std::unordered_map<std::string, int>& result, const std::pair<std::string, int>& pair) {
        result[pair.first] += pair.second;
    }

    std::unordered_map<std::string, int> result;
    std::for_each(pairs.begin(), pairs.end(), std::bind(reduce, std::ref(result), std::placeholders::_1));

    return result;
}

/// @brief Pure function to filter words from a word list
/// @param wordList The list of all words to filter
/// @param filterList The list of words to filter out
/// @return The filtered list of words
auto filterWords = [](const std::vector<std::string>& wordList, const std::vector<std::string>& filterList) {
    std::vector<std::string> result;

    for (const std::string& word : wordList) {
        bool match = false;
        for (const std::string& filter : filterList) {
            if (word == filter) {
                match = true;
                break;
            }
        }
        if (match) {
            result.push_back(word);
        }
    }

    return result;
};


// Pure function to read files
auto readFile = [](const std::string& fileName) -> std::string {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + fileName);
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
};

// Pure function to tokenize text
auto tokenize = [](const std::string& inputText) {
    // Replace "CHAPTER <number>" with "CHAPTER_<number>"
    std::regex chapterPattern(R"(CHAPTER (\d+))");
    std::string processedText = std::regex_replace(inputText, chapterPattern, "CHAPTER_$1");
    
    std::istringstream stream(processedText);
    const std::vector<std::string> tokens((std::istream_iterator<std::string>(stream)), std::istream_iterator<std::string>());

    // Map step: Transform tokens
    const auto filteredTokens = [&tokens]() {
        std::vector<std::string> result;
        std::transform(tokens.begin(), tokens.end(), std::back_inserter(result),
                       [](const std::string& token) {
                           std::string filtered;
                           std::copy_if(token.begin(), token.end(), std::back_inserter(filtered),
                                        [](char c) { return std::isalpha(c) || std::isdigit(c) || c == '_'; });
                           return filtered;
                       });
        return result;
    }();

    // Reduce step: Filter out empty tokens
    const auto nonEmptyTokens = [&filteredTokens]() {
        std::vector<std::string> result;
        std::copy_if(filteredTokens.begin(), filteredTokens.end(), std::back_inserter(result),
                     [](const std::string& token) { return !token.empty(); });
        return result;
    }();

    return nonEmptyTokens;
};

//test
//
auto writeToFile = [](const std::vector<std::string>& content, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    std::copy(content.begin(), content.end(), std::ostream_iterator<std::string>(file, "\n"));
};
//
//test end

int main() {
    try {
        const std::string bookFilename = "war_and_peace.txt";
        const std::string warTermsFilename = "war_terms.txt";
        const std::string peaceTermsFilename = "peace_terms.txt";

        const auto bookContent = readFile(bookFilename);
        const auto warTerms = readFile(warTermsFilename);
        const auto peaceTerms = readFile(peaceTermsFilename);

        const auto tokenizedBookContent = tokenize(bookContent);
        const auto tokenizedWarTerms = tokenize(warTerms);
        const auto tokenizedPeaceTerms = tokenize(peaceTerms);
        
        //test
        //
        writeToFile(tokenizedBookContent, "tokenized_book.txt");
        //
        //test end

        
        // Filter out war and peace terms from the book content
        const auto filteredWarContent = filterWords(tokenizedBookContent, tokenizedWarTerms);
        const auto filteredPeaceContent = filterWords(tokenizedBookContent, tokenizedPeaceTerms);

        // Count occurences of war and peace terms in the book content
        const auto warCounts = countOccurences(filteredWarContent);
        const auto peaceCounts = countOccurences(filteredPeaceContent);

        // print results
        std::cout << "War counts: " << std::endl;
        for (const auto& pair : warCounts) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Peace counts: " << std::endl;
        for (const auto& pair : peaceCounts) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
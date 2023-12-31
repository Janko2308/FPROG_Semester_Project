#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <regex>
#include <numeric>
#include <map>
#include <unordered_map>
#include <functional>
#include <optional>

/// @brief Pure function to calculate the distances between occurences of words
/// @param occurences A map of words to their positions in the text
/// @return A map of words to their distances between occurences
auto calculateDistances = [](const std::unordered_map<std::string, int>& occurences) {
    std::map<std::string, std::vector<int>> distances;

    // for each entry in occurences, initialize a vector dist, the size is the count of the word
    // count of the word is the value of how many times the word appeared
    // fill the vector with 0, 1, 2, 3, ..., count - 1, representing indices of the word
    std::for_each(occurences.begin(), occurences.end(), [&](const auto& entry) {
        const std::string& word = entry.first;
        const int count = entry.second;

        std::vector<int>& dist = distances[word];
        dist.resize(count);

        std::iota(dist.begin(), dist.end(), 0);
    });

    // for each entry in occurences, retrieve the vector dist from the distances map
    // transform the values in dist, with respective indices, converting the distance vector
    // to a vector of distances
    std::for_each(occurences.begin(), occurences.end(), [&](const auto& entry) {
        const std::string& word = entry.first;

        std::vector<int>& dist = distances[word];
        int index = 0;
        std::transform(dist.begin(), dist.end(), dist.begin(), [&index](int) { return index++; });
    });

    return distances;
};

/// @brief Pure function to calculate the density of a word in a chapter
/// @param occurrences A map of words to their counts
/// @param totalWordsInChapter The total number of words in the chapter
/// @return The density of the word in the chapter
auto calculateDensity = [](const std::unordered_map<std::string, int>& occurrences, int totalWordsInChapter) {
    double totalOccurrences = std::accumulate(occurrences.begin(), occurrences.end(), 0,
        [](const int previous, const std::pair<std::string, int>& p) { return previous + p.second; });
    return totalWordsInChapter > 0 ? totalOccurrences / totalWordsInChapter : 0.0;
};

/// @brief Pure function to count occurences of words in a word list
/// @param words The list of words to count
/// @return A map of words to their counts
auto countOccurences = [](const std::vector<std::string>& words) {
    // Map step: Transform words into pairs of (word, 1)
    // As such all pairs are initialized with a count of 1
    auto map = [](const std::string& word) {
        return std::make_pair(word, 1);
    };

    // Transform the words into pairs
    std::vector<std::pair<std::string, int>> pairs;
    std::transform(words.begin(), words.end(), std::back_inserter(pairs), map);

    // Reduce step: Reduce the pairs into a map of words to their counts
    auto reduce = [](std::unordered_map<std::string, int>& result, const std::pair<std::string, int>& pair) {
        result[pair.first] += pair.second;
    };

    // Iterate over each element in the pairs vector and reduce them using reduce function
    // as such updating the counts of words in the result map.
    std::unordered_map<std::string, int> result;
    std::for_each(pairs.begin(), pairs.end(), std::bind(reduce, std::ref(result), std::placeholders::_1));

    return result;
};

/// @brief Pure function to filter words from a word list
/// @param wordList The list of all words to filter
/// @param filterList The list of words to filter out
/// @return The filtered list of words
auto filterWords = [](const std::vector<std::string>& filterList) {
    return [filterList](const std::vector<std::string>& wordList) {
        std::vector<std::string> result;

        // if the word from wordList is in filterList, copy it to result
        std::copy_if(wordList.begin(), wordList.end(), std::back_inserter(result), [&filterList](const std::string& word) {
            return std::find(filterList.begin(), filterList.end(), word) != filterList.end();
        });

        return result;
    };
};



// Pure function to read files
/// @brief Read file contents into a string
/// @param fileName The name of the file to read
/// @return The contents of the file
auto readFile = [](const std::string& fileName) -> std::optional<std::string> {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        return std::nullopt;
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
};

/// @brief Tokenize the input text
/// @param optionalInputText The input text to tokenize
/// @return A vector of tokens
auto tokenize = [](const std::optional<std::string>& optionalInputText) -> std::vector<std::string> {
    if (!optionalInputText) {
        return {}; // Return an empty vector if there's no input text
    }

    const std::string& inputText = *optionalInputText;
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

/// @brief Split the tokens by chapter
/// @param tokens The tokens to split
/// @return A map of chapter numbers to their tokens
auto splitByChapter = [](const std::vector<std::string>& tokens) {
    std::map<int, std::vector<std::string>> chapters;
    std::regex chapterPattern(R"(CHAPTER_\d+)");
    int chapterIndex = 0;

    // Use std::for_each to iterate over the tokens
    std::for_each(tokens.begin(), tokens.end(), [&chapters, &chapterIndex, &chapterPattern](const std::string& token) {
        if (std::regex_match(token, chapterPattern)) {
            // Start a new chapter
            chapterIndex++;
        } else {
            // Add token to the current chapter's vector
            chapters[chapterIndex].push_back(token);
        }
    });

    // If the first token is not a chapter and chapterIndex is still 0, remove the entry.
    if (chapterIndex == 0) {
        chapters.erase(chapterIndex);
    }

    return chapters;
};

int main() {
    const std::string bookFilename = "war_and_peace.txt";
    const std::string warTermsFilename = "war_terms.txt";
    const std::string peaceTermsFilename = "peace_terms.txt";

    const auto bookContent = readFile(bookFilename);
    const auto warTerms = readFile(warTermsFilename);
    const auto peaceTerms = readFile(peaceTermsFilename);

    const auto tokenizedBookContent = tokenize(bookContent);
    const auto chapters = splitByChapter(tokenizedBookContent);
    
    const auto tokenizedWarTerms = tokenize(warTerms);
    const auto tokenizedPeaceTerms = tokenize(peaceTerms);

    std::map<int, double> warDensities;
    std::map<int, double> peaceDensities;
    // Processing each chapter
    std::for_each(chapters.begin(), chapters.end(), [&](const auto& chapterPair) {
        auto chapterNum = chapterPair.first;
        const auto& chapterContent = chapterPair.second;

        // Create filtered content
        auto filteredWarContent = filterWords(tokenizedWarTerms)(chapterContent);
        auto filteredPeaceContent = filterWords(tokenizedPeaceTerms)(chapterContent);

        // Count occurrences
        auto warCounts = countOccurences(filteredWarContent);
        auto peaceCounts = countOccurences(filteredPeaceContent);

        // Calculate densities
        double warDensity = calculateDensity(warCounts, chapterContent.size());
        double peaceDensity = calculateDensity(peaceCounts, chapterContent.size());

        // Assign chapter densities
        warDensities[chapterNum] = warDensity;
        peaceDensities[chapterNum] = peaceDensity;
    });

    // Determine the theme of each chapter based on the densities
    std::for_each(warDensities.begin(), warDensities.end(), [&](const auto& warDensityPair) {
        auto chapterNum = warDensityPair.first;
        if(chapterNum == 0) return; // Skip the the words before the first chapter
        auto warDensity = warDensityPair.second;
        auto peaceDensity = peaceDensities[chapterNum];

        std::string chapterTheme = (warDensity > peaceDensity) ? "war-related" : "peace-related";
        std::cout << "Chapter " << chapterNum << ": " << chapterTheme << std::endl;
    });

    return 0;
}





#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <regex>
#include <numeric>

/// @brief Pure function to calculate the distances between occurences of words
/// @param occurences A map of words to their positions in the text
/// @return A map of words to their distances between occurences
auto calculateDistances = [](const std::unordered_map<std::string, int>& occurences) {
    std::map<std::string, std::vector<int>> distances;

    std::for_each(occurences.begin(), occurences.end(), [&](const auto& entry) {
        const std::string& word = entry.first;
        const int count = entry.second;

        std::vector<int>& dist = distances[word];
        dist.resize(count);

        std::iota(dist.begin(), dist.end(), 0);
    });

    std::for_each(occurences.begin(), occurences.end(), [&](const auto& entry) {
        const std::string& word = entry.first;

        std::vector<int>& dist = distances[word];
        int index = 0;
        std::transform(dist.begin(), dist.end(), dist.begin(), [&index](int) { return index++; });
    });

    return distances;
};

/// @brief Pure function to calculate the density of occurences of words
/// @param occurences A map of words to their positions in the text
/// @return A map of words to their densities
auto calculateDensity = [](const std::unordered_map<std::string, int>& occurences) {
    auto distances = calculateDistances(occurences);

    std::map<std::string, double> density;

    std::transform(distances.begin(), distances.end(), std::inserter(density, density.end()), [](const auto& entry) {
        const std::vector<int>& dist = entry.second;
        double sum = std::accumulate(dist.begin(), dist.end(), 0.0);
        return std::make_pair(entry.first, sum / dist.size());
    });

    return density;
};

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
    };

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

        std::copy_if(wordList.begin(), wordList.end(), std::back_inserter(result), [&filterList](const std::string& word) {
            return std::find(filterList.begin(), filterList.end(), word) != filterList.end();
        });

        return result;
    };
};

/// @brief Read file contents into a string
/// @param fileName The name of the file to read
/// @return The contents of the file
auto readFile = [](const std::string& fileName) -> std::string {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + fileName);
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
};

/// @brief Tokenize a string into a list of words
/// @param inputText The string to tokenize
/// @return The list of words
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
        const auto filteredWarContent = filterWords(tokenizedBookContent)(tokenizedWarTerms);
        const auto filteredPeaceContent = filterWords(tokenizedBookContent)(tokenizedPeaceTerms);

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

        // Count densities of war and peace terms in the book content
        const auto warDensity = calculateDensity(warCounts);
        const auto peaceDensity = calculateDensity(peaceCounts);

        // print results
        std::cout << "War density: " << std::endl;
        for (const auto& pair : warDensity) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Peace density: " << std::endl;
        for (const auto& pair : peaceDensity) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

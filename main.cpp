#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <regex>

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

        // Output results
        std::cout << "\nTokenized War Terms:\n";
        std::copy(tokenizedWarTerms.begin(), tokenizedWarTerms.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

        std::cout << "\nTokenized Peace Terms:\n";
        std::copy(tokenizedPeaceTerms.begin(), tokenizedPeaceTerms.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

        // std::cout << "\nTokenized Book Content:\n";
        // std::copy(tokenizedBookContent.begin(), tokenizedBookContent.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

        //test
        //
        writeToFile(tokenizedBookContent, "tokenized_book.txt");
        //
        //test end

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
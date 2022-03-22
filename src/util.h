#pragma once
#include <vector>
#include <string>

class StringUtil {
public:
	std::vector<std::string> delimit(std::string str, char delimiter) {
        std::vector<std::string> delimited;
        std::string curr = "";
        for (char c : str) {

            if (c == delimiter) { 
                if (curr.size() > 0) {
                    delimited.push_back(curr); // Don't push back empty strings (double delimiter guard)
                }
                curr = "";
            }
            else {
                curr = curr + c;
            }

        }

        if (curr.size() > 0) {
            //Add the last element
            delimited.push_back(curr);
        }

        return delimited;
	}
};
#!/bin/bash

# Build the tests and the application
make test
make run

# Run the tests
echo "Running tests..."
./TextualTideTests

# If tests are successful, run the main application
if [ $? -eq 0 ]; then
    echo "Running main application..."
    ./TextualTide
else
    echo "Tests failed. Not running main application."
fi
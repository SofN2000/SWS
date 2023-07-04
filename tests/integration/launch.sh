#!/bin/sh

# Install required packages in virtualenv
python -m venv env
source env/bin/activate
python -m pip install -r tests/integration/requirements.txt

# Create files
echo "<!doctype html><html><head><title>Example Domain</title></head><body>Hello World!</body></html>" > /tmp/index.html
echo "Chemtrails man, chemtrails" > /tmp/super_top_secret.secret
chmod 000 /tmp/super_top_secret.secret

# Run testsuite
pytest

# Get testsuite exit code
code=$?

# Destroy files
chmod 777 /tmp/super_top_secret.secret
rm -f /tmp/super_top_secret.secret
rm -f /tmp/index.html

# Leave virtualenv
deactivate

# Exit with testsuite exit code
exit "$code"

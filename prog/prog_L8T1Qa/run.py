#!/usr/env python

# Argv[1] = L8 Band 4
# Argv[2] = L8 Band 5
# Argv[3] = L8 Band 6
# Argv[4] = L8 Qa Band
# Argv[5] = L8 QA Masked NDVI Output
# Argv[6] = L8 QA Masked NDWI Output

import sys
import subprocess
err=subprocess.call(['./l8t1qa',sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4],sys.argv[5]])
print err

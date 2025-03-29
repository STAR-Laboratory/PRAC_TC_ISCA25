import pandas as pd

# Read the CSV file
file_path = '../../stats/L1D-IPStride-With_RD-Merge/TPRAC-1C_4R-main_perf.csv'
df = pd.read_csv(file_path)

# Columns related to mitigation results (adjust as necessary)
# mitigation_columns = ['QPRAC-ABO-Only', 'QPRAC-BAT-RFM', 'QPRAC-TPRAC-S']
# mitigation_columns = ['ABO_RFM']
# mitigation_columns = ['ABO_Only']
mitigation_columns = ['TPRAC']
# mitigation_columns = ['ABO_RFM']

# Identify rows with missing values in any mitigation-related column
missing_rows = df[df[mitigation_columns].isnull().any(axis=1)]

# Save the rows with missing values to a new CSV file
output_path = './missing_results.csv'
missing_rows.to_csv(output_path, index=False)

print(f"Saved rows with missing results to {output_path}")
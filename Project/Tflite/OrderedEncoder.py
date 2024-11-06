import pandas as pd
from sklearn.preprocessing import LabelEncoder
from sklearn.utils import column_or_1d


class OrderedLabelEncoder(LabelEncoder):
    def fit(self, y):
        y = column_or_1d(y, warn=True)
        self.classes_ = pd.Series(y).unique()
        return self

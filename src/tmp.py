import pandas as pd
import numpy as np
from functools import partial
from time import time

def main():
    df = pd.read_stata("/tmp/St21133.000002")
    df['price_k'] = df.groupby('j')['price'].transform('std')

    timer = time()
    df['priceA3'] = df.groupby(
        ['foreign'],
        sort = False
    ).apply(
        partial(makeDifferences, var = 'price')
    ).values.flatten()
    print(time() - timer)


def makeDifferences(df_, var):
    pw = df_[var].values
    k  = df_[var + '_k'].values[0]
    return df_[var].apply(lambda x: ((np.abs(x - pw) < k) * pw).sum())


if __name__ == "__main__":
    main()

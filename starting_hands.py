# file to test all possible starting hands against n opponents
# for comparison: https://cs.indiana.edu/~kapadia/nofoldem/
# https://www.mathematrucker.com/poker/matchups_proof.php

from ACE_Cython.ace_utils import *
import datetime as dt
import pandas as pd
import time

if __name__ == '__main__':

    hand_list = unique_starting_hands()
    hand_strings = cards_to_string(hand_list)
    wins_df = pd.DataFrame(data=None, index=hand_strings)
    ties_df = pd.DataFrame(data=None, index=hand_strings)

    for opp in range(1, 10):
        start_time = time.time()
        wins_list = list()
        ties_list = list()
        for h in hand_list:
            w, t = sim_matchup([h], remove_from_deck(FULL_DECK, [h]), random_opponents=opp, num_sims=2000000)
            wins_list.append(w)
            ties_list.append(t)

        wins_df[str(opp)] = [x[0] for x in wins_list]
        ties_df[str(opp)] = [x[0] for x in ties_list]

        print('{} Done {} opponents.'.format(dt.datetime.now(), opp))
        print('Elapsed time for loop {} is {}'.format(opp, time.time() - start_time))
        # export them each loop just in case it fails
        wins_df.to_csv('wins3.csv')
        ties_df.to_csv('ties3.csv')


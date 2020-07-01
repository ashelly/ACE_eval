# this is a file to test the ace_functions cython integration for time and accuracy

from ace_functions import init_deck, hand_rank, check_accuracy, eval_hand
import time

card_strings = ['2h', '3h', '4h', '5h', '6h', '7h', '8h', '9h', 'Th', 'Jh', 'Qh', 'Kh', 'Ah',
                '2c', '3c', '4c', '5c', '6c', '7c', '8c', '9c', 'Tc', 'Jc', 'Qc', 'Kc', 'Ac',
                '2d', '3d', '4d', '5d', '6d', '7d', '8d', '9d', 'Td', 'Jd', 'Qd', 'Kd', 'Ad',
                '2s', '3s', '4s', '5s', '6s', '7s', '8s', '9s', 'Ts', 'Js', 'Qs', 'Ks', 'As']

card_dict = {s: i for s, i in zip(card_strings, range(0, 52))}
rank_dict = {0: "High Card",
             1: "One Pair",
             2: "Two Pair",
             3: "Three of a Kind",
             4: "Straight",
             5: "Flush",
             6: "Full House",
             7: "Four of a Kind",
             8: "unused",
             9: "Straight Flush"
             }

deck = init_deck()
my_hand = deck[:7]

# check some 7-card hands
sf = eval_hand(my_hand)
print(rank_dict[hand_rank(sf)])
sf1 = eval_hand(deck[1:8])
print(rank_dict[hand_rank(sf1)])

# check some 5-card hands
quads = eval_hand([deck[0], deck[13], deck[26], deck[39], deck[12]])
print(rank_dict[hand_rank(quads)])
flush = eval_hand([deck[0], deck[1], deck[5], deck[11], deck[12]])
print(rank_dict[hand_rank(flush)])

# check a 7-card hand from strings
hand_string = ['Ah', 'Kh', 'Qh', 'Th', 'Jc', '2c', '8c']
hand = [deck[card_dict[c]] for c in hand_string]
v = rank_dict[hand_rank(eval_hand(hand))]
print(v)

# check the accuracy of all 7-card combos
start = time.time()
freq = check_accuracy(deck)
print('Seconds to run = {}'.format(time.time() - start))

for i in range(10):
    print(rank_dict[i] + ': ' + str(freq[i]))

# printed results can be compared to
# https://en.wikipedia.org/wiki/Poker_probability#Frequency_of_7-card_poker_hands
# note that straight flush is royal flush + straight flush
    # Hand	Frequency
    # Royal flush	4,324
    # Straight flush (excl. royal flush)	37,260
    # Four of a kind	224,848
    # Full house	3,473,184
    # Flush	4,047,644
    # Straight	6,180,020
    # Three of a kind	6,461,620
    # Two pair	31,433,400
    # One pair	58,627,800
    # No pair	23,294,460
    # Total	133,784,560


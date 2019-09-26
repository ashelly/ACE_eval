# contains convenience functions, global variable definitions, and visualization functions

from ACE_Cython.ace_functions import init_deck, hand_rank, matchup, sim_matchup
import time
import random
import numpy as np


CARD_STRINGS = ['2h', '3h', '4h', '5h', '6h', '7h', '8h', '9h', 'Th', 'Jh', 'Qh', 'Kh', 'Ah',
                '2c', '3c', '4c', '5c', '6c', '7c', '8c', '9c', 'Tc', 'Jc', 'Qc', 'Kc', 'Ac',
                '2d', '3d', '4d', '5d', '6d', '7d', '8d', '9d', 'Td', 'Jd', 'Qd', 'Kd', 'Ad',
                '2s', '3s', '4s', '5s', '6s', '7s', '8s', '9s', 'Ts', 'Js', 'Qs', 'Ks', 'As']

CARD_DICT = {s: i for s, i in zip(CARD_STRINGS, range(0, 52))}
FULL_DECK = init_deck()

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


def deal_hands(number_of_hands, deck):
    """
    Deal random hands from a deck.
    :param number_of_hands:     Number of hands to deal
    :param deck:                Deck to deal from
    :return:                    List of hands and deck list with the cards removed.
    """
    random.shuffle(deck)
    hands = list()
    for h in range(number_of_hands):
        hands.append((deck.pop(), deck.pop()))
    return hands, deck


def remove_from_deck(deck, hand_list):
    """
    Take an existing deck and remove cards from it.
    :param deck:            list of ints deck to remove cards from
    :param hand_list:       list of card tuples to remove
    :return:                copy of deck with cards removed
    """
    new_deck = deck[:]
    for h in hand_list:
        for c in h:
            new_deck.remove(c)
    return new_deck


def _card_to_string(c_in):
    return CARD_STRINGS[FULL_DECK.index(c_in)]


def _string_to_card(c_in):
    return FULL_DECK[CARD_DICT[c_in]]


def cards_to_string(c_in):
    """
    Turn the cards into human-readable format.

    :param c_in:    list, tuple, or string of card integers
    :return:        same object as input, converted to human-readable
    """
    if isinstance(c_in, int):
        return _card_to_string(c_in)

    if isinstance(c_in, tuple):
        return tuple(map(_card_to_string, c_in))

    if isinstance(c_in, list):
        return [cards_to_string(c) for c in c_in]


def string_to_cards(cards, is_board=False):
    """
    Convert human readable cards to the ints needed by the program.

    :param cards:       string of cards, list of strings, or list of tuples of strings
                        format examples: 'AhKh7c7s', ['Ah', 'Kh', '7c', '7s'], [('Ah','Kh'), ('7c', '7s')]
                        broken up into pairs representing a hand (split in the order passed if not tuples)
    :param is_board:    if True, it is a board; represents hands if False
    :return:            if is_board=False, returns list 2-length hand tuples
                        if is_board=True, returns a board tuple
    """
    if isinstance(cards, str):
        card_list = [cards[i*2:i*2+2] for i in range(len(cards)//2)]
    elif isinstance(cards, list):
        if isinstance(cards[0], tuple):
            card_list = [c for h in cards for c in h]
        else:
            card_list = cards
    elif isinstance(cards, tuple):
        card_list = list(cards)
    else:
        raise TypeError

    if not is_board:
        c_iter = iter(card_list)
        hand_list = list()
        for a, b in zip(c_iter, c_iter):
            hand_list.append((_string_to_card(a), _string_to_card(b)))
        return hand_list
    else:
        return tuple(map(_string_to_card, card_list))


def unique_starting_hands(offset=0):
    """
    Create a list of unique starting hands that represent
    all 169 mathematically different hands (suited, unsuited, pairs)

    :param offset:      lets you adjust the suit (offset=13)

    :return:            list of hand tuples
    """

    hand_list = list()
    for i in range(13):
        for j in range(i+1, 13):
            hand_list.append((FULL_DECK[i+offset], FULL_DECK[j+offset]))

    for i in range(13):
        for j in range(i+13, 26):
            hand_list.append((FULL_DECK[i+offset], FULL_DECK[j+offset]))

    return hand_list


def matchup_from_string(cards, board=(), random_opponents=None, num_sims=None):
    """
    Run a matchup of 2 or more hands to determine win and tie percentages.
    Can be run deterministically or with simulations.

    :param cards:               string of cards, list of strings, or list of tuples of strings
                                format examples: 'AhKh7c7s', ['Ah', 'Kh', '7c', '7s'], [('Ah','Kh'), ('7c', '7s')]
                                broken up into pairs representing a hand (split in the order passed if not tuples)
    :param board:               cards already on the board, same formats as cards: string, list, or tuple
    :param random_opponents:    int for number of random opponents to play against; increase # of hands
    :param num_sims:            int for number of simulations to perform

    :return:                    two tuples: wins, ties; both of length # of hands
                                items represent the % of each hand winning or tying, in order of hands passed
                                will likely sum to more than one since multiple hands tie
    """

    hand_list = string_to_cards(cards)
    board = string_to_cards(board, is_board=True)

    if random_opponents and num_sims:
        return sim_matchup(hand_list, remove_from_deck(FULL_DECK, hand_list + [board]), board,
                           random_opponents, num_sims)
    else:
        return matchup(hand_list, remove_from_deck(FULL_DECK, hand_list + [board]), board)

# TODO: put in a utility that does hand ranges


if __name__ == '__main__':
    # initialize a deck, deal two random hands, look at results
    d = init_deck()
    h1, d1 = deal_hands(2, d)
    print(cards_to_string(h1))
    print(matchup(h1, d1))

    # all possible matchups without a board, different input formats
    w1, t1 = matchup_from_string('AsAhKsKh')
    w2, t2 = matchup_from_string(['As', 'Ah', 'Ks', 'Kh'])
    w3, t3 = matchup_from_string([('As', 'Ah'), ('Ks', 'Kh')])

    # more opponents
    w4, t4 = matchup_from_string('AsAhKsKh8c8d')

    # all possible matchups with a board, different input formats
    w5, t5 = matchup_from_string('AsAhKsKh', 'Kc7s7h')
    w6, t6 = matchup_from_string('AsAhKsKh', ('Kc', '7s', '7h'))
    w7, t7 = matchup_from_string('AsAhKsKh', ['Kc', '7s', '7h'])

    # simulate a matchup that includes a random opponent
    w8, t8 = matchup_from_string(['As', 'Ah'], (), 1, 1000000)
    # simulate a matchup with a known opponent and a random opponent
    w9, t9 = matchup_from_string(['As', 'Ah', 'Ks', 'Kh'], (), 1, 1000000)

    # simulate with board
    # TODO: probably could just run all possible combinations)
    w10, t10 = matchup_from_string(['As', 'Ah', 'Ks', 'Kh'], 'Kc7c7s', 1, 1000000)
    w11, t11 = matchup_from_string(['As', 'Ah', 'Ks', 'Kh'], ('Kc', '7c', '7s'), 1, 1000000)

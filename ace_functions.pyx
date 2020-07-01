# distutils: define_macros=CYTHON_TRACE_NOGIL=1
# cython: boundscheck=False
# cython: wraparound=False

# cython file with key functions for evaluating hands and running matchups
# needs to be compiled by running ACE_Cython.ace_setup

from libc.stdint cimport uint32_t as Card, uint16_t
from libc.stdlib cimport rand, RAND_MAX
import itertools
cimport numpy as np
import numpy as np
from scipy.special import comb

cdef extern from "ace_eval_py.h":
    Card ACE_makecard(Card i)
    Card ACE_evaluate(Card* h)
    void ACE_addcard(Card* h, Card c)

cdef ceval_hand(Card[:] hand_in):
    # evaluate a 7-card poker hand and return a value
    # the bigger value hand is the winner
    # uses external C functions from AShelly ACE_eval
    cdef Card h[5]  #TODO: !!! why only five?
    cdef Py_ssize_t i
    h[:] = [0, 0, 0, 0, 0]
    cdef Card* h_ptr = &h[0]
    for i in range(len(hand_in)):
        ACE_addcard(h_ptr, hand_in[i])
    cdef Card v = ACE_evaluate(h_ptr)
    return v

def eval_hand(hand_in):
    # convenience function to allow for accessing hand evaluation from python
    # takes an iterable containing the cards as an input
    cdef Card[:] h = np.zeros(7, dtype=np.uint32)
    cdef Py_ssize_t i
    cdef Card c
    for i, c in enumerate(hand_in):
        h[i] = c
    return ceval_hand(h)

cdef ceval_hand_batch(np.ndarray[Card, ndim=2] hands_in):
    # currently not used; idea was to make a version that does all a player's hands at once
    # trying to make a function to take a list of player hands in and evaluate here
    # ideas is to see if it speeds up the evaluation by making fewer calls from the python layer

    cdef np.ndarray ranks = np.zeros(1000000)

    for i in range(1000000):
        ranks[i] = ceval_hand(hands_in[i,:])

    return ranks

def init_deck():
    # initializes a deck using the 32-bit integer system
    # returns: list of integers comprising a deck
    # 2 through A, hearts, clubs, diamonds, spades

    deck = list()
    for i in range(52):
        deck.append(ACE_makecard(i))
    return deck

cdef shuffle_me(Card[:] deck, Py_ssize_t len_deck):
    # shuffles a deck memoryview

    cdef int r
    cdef Py_ssize_t i
    cdef Card temp
    for i in range(len_deck-1, 0, -1):
        r= <int>(<float>(rand())/RAND_MAX*i)
        temp = deck[i]
        deck[i]=deck[r]
        deck[r]=temp

def make_card(card):
    return ACE_makecard(card)

cdef chand_rank(Card r):
    return r>>28

def hand_rank(r):
    return r>>28

def check_accuracy(list deck):
    # loop through all possible seven-card combinations in the deck, determine their rank
    # return the counts of ranks for comparison to documented frequencies in order to check the accuracy

    cdef Card[:] freq = np.zeros(10, dtype=np.uint32)
    cdef Card v
    cdef Card r
    cdef Card[:] hand = np.zeros(7, dtype=np.uint32)
    cdef Py_ssize_t i = 0
    for h in itertools.combinations(deck, 7):
        for i in range(7):
            hand[i] = h[i]
        v = ceval_hand(hand)
        r = chand_rank(v)
        freq[r] += 1
    print('Processed {} MM hands'.format(sum(freq)/1e6))
    return freq

def matchup(hands, deck, board=()):
    '''
    runs through all possible combinations of hands v. each other

    hands:      list of hand tuples
    deck:       list of remaining card ints in the deck
    board:      tuple for the cards already on the board

    returns:    two numpy arrays: one with win percentages
                for each hand in order passed, and the other
                with tie percentages
    '''
    cdef Card hands_played = comb(len(deck), 5-len(board))
    cdef Py_ssize_t num_players = len(hands)
    cdef np.ndarray hand_ranks = np.zeros((hands_played, num_players), dtype=np.uint32)
    cdef Card[:,:] hand_ranks_view = hand_ranks
    cdef Card[:] h = np.zeros(7, dtype=np.uint32)
    cdef Py_ssize_t i, c, j
    cdef Py_ssize_t len_board = len(board)
    cdef Py_ssize_t num_hands = len(hands)
    cdef Card[:,:] hands_sim = np.zeros((num_hands, 2), dtype=np.uint32)

    for i in range(num_hands):
        hands_sim[i,0] = hands[i][0]
        hands_sim[i,1] = hands[i][1]

    for c in range(len_board):
        h[2+c] = board[c]

    for s, b in enumerate(itertools.combinations(deck, 5-len(board))):
        for c in range(5-len_board):
            h[2+len_board+c] = b[c]
        for i in range(num_hands):
            h[0] = hands_sim[i, 0]
            h[1] = hands_sim[i, 1]
            hand_ranks_view[s, i] = ceval_hand(h)

    return get_results(hand_ranks, num_players, hands_played)


def sim_matchup(list hands, list deck, tuple board=(), int random_opponents=1, int num_sims=1000000):
    '''
    Simulates hands and returns the winners and ties.

    hands:              list of hand tuples
    deck:               list of remaining card ints in the deck
    board:              tuple for the cards already on the board
    random_opponents:   int for the number of random opponents to play agaginst
    num_sims:           int for number of sims to perform

    returns:            two numpy arrays: one with win percentages
                        for each hand in order passed, and the other
                        with tie percentages
    '''

    cdef Py_ssize_t num_players = len(hands) + random_opponents
    cdef Py_ssize_t len_deck = len(deck)
    cdef Py_ssize_t num_cards_needed = 5-len(board) + 2*random_opponents
    cdef Py_ssize_t counter = 0
    cdef Py_ssize_t num_hands = len(hands)
    cdef Py_ssize_t len_board = len(board)

    cdef np.ndarray hand_ranks = np.zeros((num_sims, num_players), dtype=np.uint32)
    cdef Card[:,:] hand_ranks_view = hand_ranks
    cdef Py_ssize_t o
    cdef Py_ssize_t c
    cdef Py_ssize_t s, i

    cdef Card[:,:] hands_sim = np.zeros((num_players, 2), dtype=np.uint32)
    for i in range(num_hands):
        hands_sim[i,0] = hands[i][0]
        hands_sim[i,1] = hands[i][1]

    cdef Card[:] cdeck = np.zeros(len_deck, dtype=np.uint32)
    cdef Card[:] h = np.zeros(7, dtype=np.uint32)

    for c in range(len_deck):
        cdeck[c] = deck[c]

    for c in range(len_board):
        h[2+c] = board[c]

    shuffle_me(cdeck, len_deck)
    for s in range(num_sims):
        if (len_deck-counter) < num_cards_needed:
            shuffle_me(cdeck, len_deck)
            counter=0

        for o in range(random_opponents):
            hands_sim[num_hands + o, 0] = cdeck[counter+2*o]
            hands_sim[num_hands + o, 1] = cdeck[counter+1+2*o]

        for c in range(5-len_board):
            h[2+len_board+c] = cdeck[counter+2*random_opponents+c]

        counter += num_cards_needed

        for i in range(num_players):
            h[0] = hands_sim[i,0]
            h[1] = hands_sim[i,1]
            hand_ranks_view[s,i] = ceval_hand(h)

    return get_results(hand_ranks, num_players, num_sims)

cdef get_results(np.ndarray ranks, Py_ssize_t num_players, int num_sims):
    # helper function for matchup and sim_matchup that
    # takes a numpy array of hand ranks (rows) and players (columns) and
    # calculates the wins and ties (where value is greater than or equal to others)

    cdef np.ndarray winners = np.zeros((num_sims, num_players), dtype=np.uint32)
    cdef np.ndarray ties = np.zeros((num_sims, num_players), dtype=np.uint32)
    cdef np.ndarray m = ranks.max(axis=1)
    cdef np.ndarray a = np.tile(m, (num_players,1)).T
    cdef np.ndarray w = (ranks==a)
    cdef tuple w_ind = np.nonzero(np.tile(w.sum(axis=1)==1,(num_players,1)).T & w)
    winners[w_ind] = 1
    cdef tuple t_ind = np.nonzero(np.tile(w.sum(axis=1)!=1,(num_players,1)).T & w)
    ties[t_ind] = 1
    return winners.sum(axis=0)/num_sims, ties.sum(axis=0)/num_sims

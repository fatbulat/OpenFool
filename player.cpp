#include "player.h"

static const int RANK_MULTIPLIER = 100;

Player::Player(Table *table, int index, QString name)
    : QObject(table), _table(table), _name(name), _index(index)
{
}

int Player::handValue(QList<Card> hand)
{
    const int UNBALANCED_HAND_PENALTY = 200;
    const int MANY_CARDS_PENALTY = 600;
    const double bonuses[5] = {0, 0, 0.5, 0.75, 1.25}; // for cards of same rank
    int res = 0;
    int countsByRank[13] = {0}, countsBySuit[4] = {0};
    for (Card &c : hand) {
        int r = c.rank(), s = c.suit();
        res += (r == RANK_ACE ? 6 : r - 8) * RANK_MULTIPLIER;
        if (s == _table->trumpSuit())
            res += 13 * RANK_MULTIPLIER;
        countsByRank[r - 1]++;
        countsBySuit[s]++;
    }
    for (int i = 1; i <= 13; i++) {
        res += qMax(i == RANK_ACE ? 6 : i - 8, 1)
               * bonuses[countsByRank[i - 1]];
    }
    double avgSuit = 0;
    for (Card &c : hand) {
        if (c.suit() != _table->trumpSuit())
            avgSuit++;
    }
    avgSuit /= 3;
    for (int i = 0; i < 4; i++) {
        if (i != _table->trumpSuit()) {
            double dev = qAbs((countsBySuit[i] - avgSuit) / avgSuit);
            res -= UNBALANCED_HAND_PENALTY * dev;
        }
    }
    int cardsInPlay = _table->cardsRemaining();
    for (const Player *p : _table->players())
        cardsInPlay += p->_hand.length();
    cardsInPlay -= hand.length();
    double cardRatio = cardsInPlay ? hand.length() / cardsInPlay : 10.0;
    res += (0.25 - cardRatio) * MANY_CARDS_PENALTY;
    return res;
}

int Player::currentHandValue() { return handValue(_hand); }

QString Player::name() const { return _name; }

void Player::setName(const QString &name) { _name = name; }

int Player::index() const { return _index; }

QList<Card> Player::hand() const { return _hand; }

void Player::addCard(Card c) { _hand << c; }

void Player::startTurn()
{
    const double bonuses[5] = {0, 0, 1, 1.5, 2.5};
    int countsByRank[13] = {0};
    for (Card &c : _hand) {
        countsByRank[c.rank() - 1]++;
    }
    int maxVal = INT_MIN;
    int cardIdx = -1;
    for (int i = 0; i < _hand.length(); i++) {
        QList<Card> newHand = _hand;
        Card c = _hand[i];
        newHand.removeAt(i);
        int r = c.rank();
        int newVal = handValue(newHand)
                     + bonuses[countsByRank[r - 1]]
                           * (r == RANK_ACE ? 6 : r - 8) * RANK_MULTIPLIER;
        if (newVal > maxVal) {
            maxVal = newVal;
            cardIdx = i;
        }
    }
    Card c = _hand[cardIdx];
    _hand.removeAt(cardIdx);
    emit cardThrown(c);
}

void Player::throwOrDone()
{
    bool ranksPresent[13] = {false};
    for (Card c : _table->attackCards()) {
        ranksPresent[c.rank() - 1] = true;
    }
    for (Card c : _table->defenseCards()) {
        ranksPresent[c.rank() - 1] = true;
    }
    // TODO: Remove duplication
    const double bonuses[5] = {0, 0, 1, 1.5, 2.5};
    int countsByRank[13] = {0};
    for (Card &c : _hand) {
        countsByRank[c.rank() - 1]++;
    }
    int maxVal = INT_MIN;
    int cardIdx = -1;
    for (int i = 0; i < _hand.length(); i++) {
        QList<Card> newHand = _hand;
        Card c = _hand[i];
        int r = c.rank();
        if (!ranksPresent[r - 1])
            continue;
        newHand.removeAt(i);
        int newVal = handValue(newHand)
                     + bonuses[countsByRank[r - 1]]
                           * (r == RANK_ACE ? 6 : r - 8) * RANK_MULTIPLIER;
        if (newVal > maxVal) {
            maxVal = newVal;
            cardIdx = i;
        }
    }
    int PENALTY = 400;
    if (currentHandValue() - maxVal < PENALTY && cardIdx >= 0) {
        Card c = _hand[cardIdx];
        _hand.removeAt(cardIdx);
        emit cardThrown(c);
    } else {
        emit done();
    }
}

void Player::tryBeat()
{
    const int RANK_PRESENT_BONUS = 300;
    bool ranksPresent[13] = {false};
    QList<Card> handIfTake = _hand;
    for (Card c : _table->attackCards()) {
        ranksPresent[c.rank() - 1] = true;
        handIfTake.append(c);
    }
    for (Card c : _table->defenseCards()) {
        ranksPresent[c.rank() - 1] = true;
        handIfTake.append(c);
    }
    int maxVal = INT_MIN;
    int cardIdx = -1;
    Card attack = _table->attackCards().back();
    for (int i = 0; i < _hand.length(); i++) {
        Card c = _hand[i];
        if (_table->beats(c, attack)) {
            int r = c.rank();
            QList<Card> newHand = _hand;
            newHand.removeAt(i);
            int newVal = handValue(newHand)
                         + RANK_PRESENT_BONUS * ranksPresent[r - 1];
            if (newVal > maxVal) {
                maxVal = newVal;
                cardIdx = i;
            }
        }
    }
    int PENALTY = 800, TAKE_PENALTY = 900;
    if (((currentHandValue() - maxVal < PENALTY)
         || (handValue(handIfTake) - maxVal < TAKE_PENALTY
             && _table->cardsRemaining())) && cardIdx >= 0) {
        Card c = _hand[cardIdx];
        _hand.removeAt(cardIdx);
        emit cardBeaten(c);
    } else {
        emit take();
    }
}

void Player::clearHand() { _hand.clear(); }

void Player::throwCard(Card c)
{
    bool ranksPresent[13] = {false};
    for (Card c : _table->attackCards()) {
        ranksPresent[c.rank() - 1] = true;
    }
    for (Card c : _table->defenseCards()) {
        ranksPresent[c.rank() - 1] = true;
    }
    if (_hand.contains(c)
        && (ranksPresent[c.rank() - 1] || _table->attackCards().empty())) {
        _hand.removeAll(c);
        emit cardThrown(c);
    }
}

void Player::beatWithCard(Card c)
{
    Card attack = _table->attackCards().back();
    if (_hand.contains(c) && _table->beats(c, attack)) {
        _hand.removeAll(c);
        emit cardBeaten(c);
    }
}

#include "cards.h"

Cards::Cards()
{
    cards.append(new Card("R", "0"));
    cards.append(new Card("R", "1"));
    cards.append(new Card("R", "1"));
    cards.append(new Card("R", "2"));
    cards.append(new Card("R", "2"));
    cards.append(new Card("R", "3"));
    cards.append(new Card("R", "3"));
    cards.append(new Card("R", "4"));
    cards.append(new Card("R", "4"));
    cards.append(new Card("R", "5"));
    cards.append(new Card("R", "5"));
    cards.append(new Card("R", "6"));
    cards.append(new Card("R", "6"));
    cards.append(new Card("R", "7"));
    cards.append(new Card("R", "7"));
    cards.append(new Card("R", "8"));
    cards.append(new Card("R", "8"));
    cards.append(new Card("R", "9"));
    cards.append(new Card("R", "9"));
    cards.append(new Card("B", "0"));
    cards.append(new Card("B", "1"));
    cards.append(new Card("B", "1"));
    cards.append(new Card("B", "2"));
    cards.append(new Card("B", "2"));
    cards.append(new Card("B", "3"));
    cards.append(new Card("B", "3"));
    cards.append(new Card("B", "4"));
    cards.append(new Card("B", "4"));
    cards.append(new Card("B", "5"));
    cards.append(new Card("B", "5"));
    cards.append(new Card("B", "6"));
    cards.append(new Card("B", "6"));
    cards.append(new Card("B", "7"));
    cards.append(new Card("B", "7"));
    cards.append(new Card("B", "8"));
    cards.append(new Card("B", "8"));
    cards.append(new Card("B", "9"));
    cards.append(new Card("B", "9"));
    cards.append(new Card("J", "0"));
    cards.append(new Card("J", "1"));
    cards.append(new Card("J", "1"));
    cards.append(new Card("J", "2"));
    cards.append(new Card("J", "2"));
    cards.append(new Card("J", "3"));
    cards.append(new Card("J", "3"));
    cards.append(new Card("J", "4"));
    cards.append(new Card("J", "4"));
    cards.append(new Card("J", "5"));
    cards.append(new Card("J", "5"));
    cards.append(new Card("J", "6"));
    cards.append(new Card("J", "6"));
    cards.append(new Card("J", "7"));
    cards.append(new Card("J", "7"));
    cards.append(new Card("J", "8"));
    cards.append(new Card("J", "8"));
    cards.append(new Card("J", "9"));
    cards.append(new Card("J", "9"));
    cards.append(new Card("V", "0"));
    cards.append(new Card("V", "1"));
    cards.append(new Card("V", "1"));
    cards.append(new Card("V", "2"));
    cards.append(new Card("V", "2"));
    cards.append(new Card("V", "3"));
    cards.append(new Card("V", "3"));
    cards.append(new Card("V", "4"));
    cards.append(new Card("V", "4"));
    cards.append(new Card("V", "5"));
    cards.append(new Card("V", "5"));
    cards.append(new Card("V", "6"));
    cards.append(new Card("V", "6"));
    cards.append(new Card("V", "7"));
    cards.append(new Card("V", "7"));
    cards.append(new Card("V", "8"));
    cards.append(new Card("V", "8"));
    cards.append(new Card("V", "9"));
    cards.append(new Card("V", "9"));
    cards.append(new Card("R", "+2"));
    cards.append(new Card("R", "+2"));
    cards.append(new Card("B", "+2"));
    cards.append(new Card("B", "+2"));
    cards.append(new Card("J", "+2"));
    cards.append(new Card("J", "+2"));
    cards.append(new Card("V", "+2"));
    cards.append(new Card("V", "+2"));
    cards.append(new Card("R", "I"));
    cards.append(new Card("R", "I"));
    cards.append(new Card("B", "I"));
    cards.append(new Card("B", "I"));
    cards.append(new Card("J", "I"));
    cards.append(new Card("J", "I"));
    cards.append(new Card("V", "I"));
    cards.append(new Card("V", "I"));
    cards.append(new Card("R", "P"));
    cards.append(new Card("R", "P"));
    cards.append(new Card("B", "P"));
    cards.append(new Card("B", "P"));
    cards.append(new Card("J", "P"));
    cards.append(new Card("J", "P"));
    cards.append(new Card("V", "P"));
    cards.append(new Card("V", "P"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "J"));
    cards.append(new Card("N", "J"));
    cards.append(new Card("N", "J"));
    cards.append(new Card("N", "J"));
}

Card* Cards::get(int i) const
{
    return cards.at(i);
}

int Cards::size() const
{
    return cards.size();
}

void Cards::remove(int i)
{
    cards.removeAt(i);
}

bool Cards::isEmpty() const
{
    return cards.isEmpty();
}

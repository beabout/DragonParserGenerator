#include <iostream>
#include <list>
#include <map>
#include <string>
#include <fstream>
#include <ostream>
using namespace std;

class Item;
class Edge;
class ParserState;
list<Edge> global_edges;

void printMap(map<string, list<string>> m);
void writeToDOT(ParserState *state);
void generateSubStates(ParserState *state, int &counter, list<string> keys, map<string, list<string>> amap);

class Item
{
public:
  Item() : myKey(""){};
  Item(string key, list<string> lhs, list<string> rhs) : myKey(key), myLHS(lhs), myRHS(rhs){};
  string getKey() { return myKey; }
  list<string> getLHS() { return myLHS; }
  list<string> getRHS() { return myRHS; }
  void addKey(string key)
  {
    myKey = key;
  }
  void addLHS(list<string> left)
  {
    myLHS = left;
  }
  void addRHS(list<string> right)
  {
    myRHS = right;
  }
  void setRHSIsTerminal(bool term)
  {
    rhs_is_terminal = term;
  }
  bool hasTerminalRHS()
  {
    return rhs_is_terminal;
  }

  string lhs_to_string()
  {
    string s = "";
    for (auto str : myLHS)
    {
      s += str;
    }
    return s;
  }
  string rhs_to_string()
  {
    string s = "";
    for (auto str : myRHS)
    {
      s += str;
    }
    return s;
  }

private:
  bool rhs_is_terminal;
  string myKey;
  list<string> myLHS;
  list<string> myRHS;
};

class Edge
{
public:
  Edge(ParserState *src, ParserState *tgt, string token, bool isTerminal) : mySrc(src), myTgt(tgt), myToken(token), myTokenIsTerminal(isTerminal){};
  ParserState *getSrc() { return mySrc; }
  ParserState *getTgt() { return myTgt; }
  string getToken() { return myToken; }
  bool tokenIsTerminal() { return myTokenIsTerminal; }

private:
  ParserState *mySrc;
  ParserState *myTgt;
  string myToken;
  bool myTokenIsTerminal;
};

class ParserState
{
public:
  ParserState(int id) : myID(id){};
  const int getID() { return myID; }
  void to_dot(ostream &out)
  {
    string s = "\t\"state" + to_string(myID) + "\" [ style = \"filled\" penwidth = 1 fillcolor = \"white\" fontname = \"Courier New\" shape = \"Mrecord\" label =<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\"><tr><td bgcolor=\"black\" align=\"center\" colspan=\"2\"><font color=\"white\"> State #" + to_string(myID) + "</font></td></tr>";
    for (auto item : items)
    {
      s += "<tr><td align=\"left\" port=\"r2\">";
      s += item->getKey() + " -&gt; " + item->lhs_to_string() + " &bull; " + item->rhs_to_string();
      s += "</td></tr>";
    }
    s += "</table>> ];\n";
    out << s;
    for (auto edge : outgoingEdges)
    {
      edge.getTgt()->to_dot(out);
    }
  }
  bool parseTokens(list<string> tokenstream)
  {
    bool passing = false;
    if (0 < tokenstream.size())
    {
      for (auto edge : outgoingEdges)
      {
        if (edge.getToken() == tokenstream.front() && edge.tokenIsTerminal())
        {
          tokenstream.pop_front();
          passing = edge.getTgt()->parseTokens(tokenstream) || passing;
          return passing;
        }
      }
    }
    else
    {
      return true;
    }
  }
  void addItem(Item *item)
  {
    items.push_back(item);
  }
  void addOutgoingEdge(Edge edge)
  {
    outgoingEdges.push_back(edge);
  };
  const list<Item *> getItems()
  {
    return items;
  }
  bool previousItemsRHSIncludesValue(string val)
  {
    bool included = false;
    if (items.size() > 0)
    {
      cout << val << endl;
      cout << "in here\n";
      for (auto item : items)
      {
        cout << "in here2\n";
        if (item->getRHS().front() == val)
        {
          cout << "in here3\n";
          included = true;
        }
      }
    }
    return included;
  }

private:
  list<Edge> outgoingEdges;
  list<Item *> items;
  const int myID;
};

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Yooo hooo! I expected an input token stream file (ex. `test.tokens`)\n";
    return 0;
  }
  map<string, list<string>> amap;
  amap["A"].push_back("a B");
  amap["B"].push_back("C");
  amap["B"].push_back("hello");
  amap["B"].push_back("w o rld");

  map<string, list<string>> temp;
  amap["A"].push_back("a B");
  amap["B"].push_back("C");
  amap["B"].push_back("hello");
  amap["B"].push_back("w o rld");

  int id_counter = 1;
  list<string> keys;

  // first state
  ParserState *initial = new ParserState(id_counter);
  id_counter++;
  list<string> empty_list;
  list<string> starter_item_elements;
  starter_item_elements.push_back(amap.begin()->first);
  Item *starter = new Item("S", empty_list, starter_item_elements);
  initial->addItem(starter);
  for (auto it = amap.begin(); it != amap.end(); ++it)
  {
    if (initial->previousItemsRHSIncludesValue(it->first))
    {
      keys.push_front(it->first);
      Item *thing = new Item();
      for (auto item : it->second)
      {
        list<string> right_hand_side;
        thing->addKey(it->first);
        // if item contains " " and more.
        if (item.find(" ") != string::npos)
        {
          string temp = item;
          while (temp.find(" ") != string::npos)
          {
            string next_item = temp.substr(0, temp.find(" "));
            right_hand_side.push_back(next_item);
            temp = temp.erase(0, temp.find(" ") + 1);
          }
          right_hand_side.push_back(temp);
        }
        else
        {
          right_hand_side.push_back(item);
        }
        thing->addLHS(empty_list);
        thing->addRHS(right_hand_side);
      }
      initial->addItem(thing);
      amap.erase(it->first);
    }
  }
  amap.erase("S");
  generateSubStates(initial, id_counter, keys, amap);

  // Ok. Let's get to business. Let's see if we can accept the tokens.
  ifstream inFile;
  inFile.open(argv[1]);
  list<string> stream;
  string token = "";
  while (getline(inFile, token))
  {
    if (token.find(":") != string::npos)
    {
      token = token.substr(0, (token.find(":")));
    }
    else if (token.find("[") != string::npos && token.find("]") != string::npos)
    {
      token = token.substr(0, (token.find("[") - 1));
    }
    else
    {
      cout << "\nline had neither [] or :\n";
    }
    if (token == " ")
    {
      token = "~";
    }
    stream.push_back(token);
  }
  cout << "\nTesting input token stream: ";

  int exit_status = 0;
  if (initial->parseTokens(stream))
  {
    cout << "Accepted";
  }
  else
  {
    exit_status = 1;
    cout << "Rejected";
  }
  cout << "\n\n";

  writeToDOT(initial);

  return exit_status;
}

void printMap(map<string, list<string>> m)
{
  for (auto it = m.begin(); it != m.end(); ++it)
  {
    cout << it->first;
    cout << ": ";
    for (auto item : it->second)
    {
      cout << item << " | ";
    }
    cout << "\n";
  }
}

void writeToDOT(ParserState *state)
{
  ofstream outStream("parser.dot");
  outStream << "digraph automaton {\n";
  outStream << "\tgraph[rankdir = \"LR\"]\n";
  outStream << "\tratio = auto;\n";
  // Pass outstream into initial ParserState to_dot(out);
  // The following two lines are examples of states
  state->to_dot(outStream);
  // This is an example of an edge
  for (auto edge : global_edges)
  {
    outStream << "\tstate" + to_string(edge.getSrc()->getID()) + "->state" + to_string(edge.getTgt()->getID()) + "[penwidth = 5 fontcolor = \"black\" label = \"" + edge.getToken() + "\"];\n";
  }

  outStream << "}\n";
  outStream.close();
}

void generateSubStates(ParserState *currentState, int &counter, list<string> keys, map<string, list<string>> amap)
{
  for (auto item : currentState->getItems())
  {
    cout << "key: " << item->getKey() << endl;
    bool isTerminal = true;
    // if there is still stuff to be discovered.
    if (item->getRHS().size() > 0)
    {
      // 1. generate a state if necessary
      ParserState *newState = new ParserState(counter);
      counter++;
      string edge_key = item->getRHS().front();
      list<string> new_lhs = item->getLHS();
      list<string> new_rhs = item->getRHS();

      new_lhs.push_back(edge_key);
      new_rhs.pop_front();
      Item *im = new Item(item->getKey(), new_lhs, new_rhs);
      newState->addItem(im);

      Edge tie = Edge(currentState, newState, edge_key, isTerminal);
      currentState->addOutgoingEdge(tie);
      global_edges.push_back(tie);

      std::map<string, list<string>>::iterator it = amap.begin();
      // while(it != amap.end()){
      if (item->getRHS().front() == it->first)
      {
        for (auto rhs : it->second)
        {
          cout << rhs << endl;
          // ["C", "hello", "w o rld"]
          Item *thing = new Item();
          thing->addKey(it->first);
          thing->addLHS(list<string>());
          list<string> new_rhs = list<string>();
          if (rhs.find(" ") != string::npos)
          {
            string temp = rhs;
            while (temp.find(" ") != string::npos)
            {
              string next_item = temp.substr(0, temp.find(" "));
              new_rhs.push_back(next_item);
              temp = temp.erase(0, temp.find(" ") + 1);
            }
            new_rhs.push_back(temp);
          }
          else
          {
            new_rhs.push_back(rhs);
          }
          thing->addRHS(new_rhs);
          newState->addItem(thing);
        }
      }
      // ++it;
      // }

      for (auto key : keys)
      {
        if (key == edge_key)
        {
          isTerminal = false;
        }
      }
      // 2. call generateSubStates on that state.
      // generateSubStates(newState, counter, keys, amap);
    }
  }
}

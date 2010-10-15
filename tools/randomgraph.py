import random, sys


def MakeNodes(numNodes):
  letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  nodes = set([])
  while len(nodes) < numNodes:
    node = random.choice(letters) + random.choice(letters) + random.choice(letters)
    nodes.add(node)
  nodes = tuple(nodes)
  return nodes


def MakeEdges(nodes, numEdges):
  edges = {}
  while numEdges > 0:
    fromNode = random.choice(nodes)
    toNode = random.choice(nodes)

    # Reject self edges.
    if fromNode == toNode:
      continue

    # Reject duplicate edges.
    if fromNode in edges and toNode in edges[fromNode]:
      continue
    if toNode in edges and fromNode in edges[toNode]:
      continue

    if fromNode not in edges:
      edges[fromNode] = set([])
    if toNode not in edges:
      edges[toNode] = set([])

    edges[fromNode].add(toNode)
    numEdges -= 1

  return edges


def PrintGraph(edges, filename):
  with open(filename, "w") as f:
    for fromNode, toNodes in edges.iteritems():
      for toNode in toNodes:
        print >> f, fromNode + toNode


def PrintNodes(nodes, numPaths, numStarts, filename):
  starts = set([])
  while len(starts) < numStarts:
    starts.add(random.choice(nodes))

  with open(filename, "w") as f:
    print >> f, numStarts
    for n in starts:
      print >> f, n


def main():
  if len(sys.argv) != 6:
    print >> sys.stderr, "Usage: %s <nodes> <edges> <paths> <starts> <basename>" % sys.argv[0]
    sys.exit(-1)

  numNodes = int(sys.argv[1])
  numEdges = int(sys.argv[2])
  numPaths = int(sys.argv[3])
  numStarts = int(sys.argv[4])
  graphName = "%s-graph.txt" % sys.argv[5]
  nodesName = "%s-nodes.txt" % sys.argv[5]

  if numStarts > numNodes:
    raise Exception("You ask too much dahlink!")

  nodes = MakeNodes(numNodes)
  edges = MakeEdges(nodes, numEdges)
  PrintGraph(edges, graphName)
  PrintNodes(nodes, numPaths, numStarts, nodesName)


if __name__ == '__main__':
  main()


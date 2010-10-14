import os, sys


def main():
  if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: %s <graph>" % sys.argv[0]
    sys.exit(-1)
  
  outname = os.path.splitext(os.path.basename(sys.argv[1]))[0]
  tmp = '.tmp-' + outname + '.dot'

  try:
    with open(tmp, 'w') as out:
      with open(sys.argv[1]) as f:
        print >> out, 'graph "%s" {'
        for line in f:
          line = line.strip()
          fromNode, toNode = line[:3], line[3:]
          print >> out, "%s -- %s;" % (fromNode, toNode)
        print >> out, '}'

    pdfname = outname + '.pdf'
    dotcmd = 'dot -Tpdf -o%s %s' % (pdfname, tmp)
    ret = os.system(dotcmd)
    if ret != 0:
      raise Exception("dot exited with return code %d" % ret)
    print >> sys.stderr, "%s created" % pdfname
  finally:
    if os.path.exists(tmp):
      os.remove(tmp)


if __name__ == '__main__':
  main()



import socket

fd = open("top-1m.csv", 'r')
hosts = fd.readlines()
fd.close()

out_file = open('ipv6-top-1m.csv', 'w')

unresolved = open('unresolved.csv', 'w')

ip_count = 0
for hostname in hosts:
    name = hostname.split(',')[1][:-1]
    node = 'www.' + name
    try:
        alladdr = socket.getaddrinfo('www.' + hostname.split(',')[1][:-1], 80, socket.AF_INET6)
    
        #Read IpV6 address
        addr = alladdr[0][4][0]

        out_file.write(addr + ', ')
        ip_count = ip_count + 1
    except Exception:
        print "Error querying node %s" % node
        unresolved.write(name + ', ')


out_file.close()
unresolved.close()

log = open('log.txt', 'w')
log.write('ips queried: ' + str(len(hosts)) + '\n')
log.write('ips fetched: ' + str(ip_count))
log.close()
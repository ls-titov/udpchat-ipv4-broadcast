#!/bin/bash
# UDP Chat setup
echo "Setting up UDP-chat..."

# Open UDP port
sudo ufw allow 12345/udp 2>/dev/null

# Set permissions if program exists
if [ -f "./udpchat" ]; then
    sudo setcap cap_net_broadcast+ep ./udpchat
fi

echo "Done! Run: make && ./udpchat 0.0.0.0 12345"

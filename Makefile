include .secrets

launch-server: backup-db kill-server
	(echo rm -rf ~/dutch_words; echo mkdir ~/dutch_words) | ssh ${PI_USERNAME}@${PI_IP_ADDRESS}
	scp -r server/ ${PI_IP_ADDRESS}:~/dutch_words/
	(echo cd ~/dutch_words/server; echo bash launch_server.sh) | ssh ${PI_USERNAME}@${PI_IP_ADDRESS}
	
kill-server:
	-(echo pkill -f server.py) | ssh ${PI_USERNAME}@${PI_IP_ADDRESS}

backup-db:
	scp -r ${PI_IP_ADDRESS}:~/dutch_words/server/dutch.db server/dutch.db

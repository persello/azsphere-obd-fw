#include "TCPIO.h"

/*size_t bufferSize, unsigned int port, void(*processData)(buffer_t*)*/
int initSocket() {



	struct ifaddrs* id;
	int val;
	val = getifaddrs(&id);
	Log_Debug("Network Interface Name :- %s\n", id->ifa_name);
	Log_Debug("Network Address of %s :- %d\n", id->ifa_name, id->ifa_addr);
	Log_Debug("Network Data :- %d \n", id->ifa_data);
	Log_Debug("Socket Data : -%c\n", id->ifa_addr->sa_data);




	struct sockaddr_in ServerIp;
	int sock = 0, client_conn = 0;
	char data_send[5] = "abcde";
	sock = socket(AF_INET, SOCK_STREAM, 0);
	//AF_INET : The address family value for IPv4 addresses
	// SOCK_STREAM argument confirms that we are using TCP as a protocol in this communication


	/* struct sockaddr : a generic socket address structure that is used for all addressing types.
	 * Often, we'll cast these to a struct sockaddr_in since we are only concerned with IPv4 addresses.
	 * ServerIp is used to store the details of server
	 * */
	memset(&ServerIp, '0', sizeof(ServerIp));
	//https://stackoverflow.com/questions/13327155/memset-definition-and-use

	ServerIp.sin_family = AF_INET;
	ServerIp.sin_port = htons(2400);
	ServerIp.sin_addr.s_addr = inet_addr("192.168.137.222");


	if (bind(sock, (struct sockaddr*) & ServerIp, sizeof(ServerIp)) == -1)
		Log_Debug("\n Socket binding failed ");

	if (listen(sock, 20) == -1)
		Log_Debug("Error\n");
	else
		Log_Debug("\n Server started\n");

	//http://man7.org/linux/man-pages/man2/accept.2.html

	client_conn = accept(sock, (struct sockaddr*)NULL, NULL);
	Log_Debug(" A request received from client \n");
	Log_Debug(" Answering with abcde ");
	write(client_conn, data_send, sizeof(data_send));
	close(client_conn);
}

int endSocket() {

}

int sendData(char* data) {

}
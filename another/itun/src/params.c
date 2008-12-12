
#include "connections.h"
#include "packets.h"
#include "data.h"

#include "params.h"

struct init_params* params_new(void)
{
	struct init_params *params = (struct init_params *) malloc(sizeof(struct init_params));
	bzero(params, sizeof(struct init_params));

	params->client_buffer	= data_new();
	params->connections	= connections_new();
	params->packets_receive	= packets_new();
	params->packets_send	= packets_new();

	return params;
}

void params_free(struct init_params *params)
{
	if (params->bind_address != NULL)
		free(params->bind_address);
	if (params->bind_port != NULL)
		free(params->bind_port);

	if (params->proxy_address != NULL)
		free(params->proxy_address);

	if (params->destination_address != NULL)
		free(params->destination_address);
	if (params->destination_port != NULL)
		free(params->destination_port);

	if (params->packets_send != NULL)
		packets_free(params->packets_send);
	if (params->packets_receive != NULL)
		packets_free(params->packets_receive);
	if (params->connections != NULL)
		connections_free(params->connections);
	if (params->client_buffer != NULL)
		data_free(params->client_buffer);

	if (params->libnet != NULL)
		libnet_destroy(params->libnet);
	if (params->libpcap != NULL)
		pcap_close(params->libpcap);

	free(params);
}

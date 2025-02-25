#pragma pack(push)
#pragma pack(1)

/// Forward declaration
namespace RakNet
{
	class BitStream;
};

#pragma pack(push, 1)
struct PlayerID
{
	unsigned int binaryAddress;
	unsigned short port;

	PlayerID& operator = (const PlayerID& input)
	{
		binaryAddress = input.binaryAddress;
		port = input.port;
		return *this;
	}

	bool operator==(const PlayerID& right) const;
	bool operator!=(const PlayerID& right) const;
	bool operator > (const PlayerID& right) const;
	bool operator < (const PlayerID& right) const;
};

struct RPCParameters
{
	unsigned char* input;
	unsigned int numberOfBitsOfData;
	PlayerID sender;
	void* recipient;
	RakNet::BitStream* replyToSender;
};

struct RPCNode
{
#pragma pack(1)
	uint8_t uniqueIdentifier;
	void(*staticFunctionPointer) (RPCParameters* rpcParms);
};
#pragma pack(pop)
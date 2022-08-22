class Client:
    def new_request(self, payload):
        raise NotImplemented()

    async def call(self, key, *args):
        response = await self.new_request(key.encode(*args))

        if type(response) == bytes:
            return key.decode(response)
        else:
            return response

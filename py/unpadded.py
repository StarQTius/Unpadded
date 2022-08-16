class Client:
    def new_request(self, payload):
        raise NotImplemented()

    async def call(self, key, *args):
        return key.decode(await self.new_request(key.encode(*args)))

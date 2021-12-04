#include <iostream>
#include <string>
#include <cstring>

template<int N>
class BlockDevice {
	private:
		uint8_t (*blocks)[N];
		uint32_t sizeInBlocks;
		
	public:
		BlockDevice(uint32_t sizeInBlocks): sizeInBlocks(sizeInBlocks) {
			this->blocks = (uint8_t (*)[N])malloc(sizeof(*blocks));
		}

		uint32_t getBlockSize() {
			return N;
		}

		uint32_t getSizeInBlocks() {
			return this->sizeInBlocks;
		}

		uint32_t getSize() {
			return N * this->sizeInBlocks;
		}
		
		void readBlock(uint32_t block, uint8_t* buffer) {
			if (block > this->sizeInBlocks)
				return;
				
			memcpy(buffer, &this->blocks[block], this->blockSize);
		}

		void writeBlock(uint32_t block, uint8_t* buffer) {
			memset(buffer, 0, this->sizeInBlocks);
			if (block > this->sizeInBlocks)
				return;

			memcpy(&this->blocks[block], buffer, this->sizeInBlocks);
		}

		~BlockDevice() {
			free(this->blocks);
		}
}

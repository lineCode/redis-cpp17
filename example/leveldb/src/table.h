#pragma once
#include "status.h"
#include "block.h"
#include "iterator.h"

class Block;
class BlockHandle;
class Footer;
struct Options;
class PosixMmapReadableFile;
struct ReadOptions;
class TableCache;

// A Table is a sorted map from strings to strings.  Tables are
// immutable and persistent.  A Table may be safely accessed from
// multiple threads without external synchronization.
class Table : public std::enable_shared_from_this<Table>
{
public:
	// Attempt to open the table that is stored in bytes [0..file_size)
	// of "file", and read the metadata entries necessary to allow
	// retrieving data from the table.
	//
	// If successful, returns ok and sets "*table" to the newly opened
	// table.  The client should delete "*table" when no longer needed.
	// If there was an error while initializing the table, sets "*table"
	// to nullptr and returns a non-ok status.  Does not take ownership of
	// "*source", but the client must ensure that "source" remains live
	// for the duration of the returned table's lifetime.
	//
	// *file must remain live while this Table is in use.
	static Status open(const Options &options,
		const std::shared_ptr<RandomAccessFile> &file,
		uint64_t fileSize,
		std::shared_ptr<Table> &table);

	//Table(const Table&) = delete;
	//void operator=(const Table&) = delete;

	~Table();

	// Returns a new iterator over the table contents.
	// The result of NewIterator() is initially invalid (caller must
	// call one of the Seek methods on the iterator before using it).
	std::shared_ptr<Iterator> newIterator(const ReadOptions &options) const ;

	// Given a key, return an approximate byte offset in the file where
	// the data for that key begins (or would begin if the key were
	// present in the file).  The returned value is in terms of file
	// bytes, and so includes effects like compression of the underlying data.
	// E.g., the approximate offset of the last key in the table will
	// be close to the file length.
	uint64_t approximateOffsetOf(const std::string_view &key) const;
	
	// Calls (*handle_result)(arg, ...) with the entry found after a call
	// to Seek(key).  May not make such a call if filter policy says
	// that key is not present.
	Status internalGet(
		const ReadOptions &options, 
		const std::string_view &key,
		const std::any &arg,
		std::function<void(const std::any &arg,
		const std::string_view &k, const std::string_view &v)> &callback);
		
	// Convert an index iterator value (i.e., an encoded BlockHandle)
	// into an iterator over the contents of the corresponding block.
	static std::shared_ptr<Iterator> blockReader(const std::any &arg,
		 const ReadOptions &options,
		 const std::string_view &indexValue);

private:
	void readMeta(const Footer &footer);
	void readFilter(const std::string_view &filterHandleValue);

	struct Rep;
	std::shared_ptr<Rep> rep;

	Table(const std::shared_ptr<Rep> &rep)
	:rep(rep)
	{
		
	}
};

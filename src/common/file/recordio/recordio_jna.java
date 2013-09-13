import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

public class recordio_jna {
    public static class RecordReader {
        public static final int RESUME_LAST_INCOMPLETE_BLOCK = 0x0002;

        protected Pointer recordReaderContext;

        public RecordReader() {
        }

        public RecordReader(String filename, int options) {
            open(filename, options);
        }

        protected void finalize() {
            close();
        }

        public boolean open(String filename, int options) {
            recordReaderContext =
                RecordIOLibrary.INSTANCE.OpenRecordReader(filename, options);
            return recordReaderContext != null;
        }

        public boolean isOpened() {
            return recordReaderContext != null;
        }

        public byte[] readRecord() {
            if (!isOpened()) {
                return null;
            }

            PointerByReference data = new PointerByReference();
            IntByReference size = new IntByReference();
            if (!RecordIOLibrary.INSTANCE.ReadRecord(recordReaderContext,
                                                     data,
                                                     size)) {
                return null;
            }

            Pointer p = data.getValue();
            byte[] buffer = p.getByteArray(0, size.getValue());
            return buffer;
        }

        public void close() {
            if (isOpened()) {
                RecordIOLibrary.INSTANCE.CloseRecordReader(recordReaderContext);
                recordReaderContext = null;
            }
        }
    }

    public static class RecordWriter {
        protected Pointer recordWriterContext;

        public RecordWriter() {
        }

        public RecordWriter(String filename,
                            int options,
                            int compression_codec) {
            open(filename, options, compression_codec);
        }

        protected void finalize() {
            close();
        }

        public boolean open(String filename,
                            int options,
                            int compression_codec) {
            recordWriterContext =
                RecordIOLibrary.INSTANCE.OpenRecordWriter(
                        filename, options, compression_codec);
            return recordWriterContext != null;
        }

        public boolean isOpened() {
            return recordWriterContext != null;
        }

        public boolean writeRecord(byte[] data) {
            if (!isOpened()) {
                return false;
            }

            return RecordIOLibrary.INSTANCE.WriteRecord(recordWriterContext,
                                                        data,
                                                        data.length);
        }

        public boolean flush() {
            if (!isOpened()) {
                return false;
            }

            return RecordIOLibrary.INSTANCE.FlushRecordWriter(
                    recordWriterContext);
        }

        public void close() {
            if (isOpened()) {
                RecordIOLibrary.INSTANCE.CloseRecordWriter(recordWriterContext);
                recordWriterContext = null;
            }
        }
    }

    private interface RecordIOLibrary extends Library {
        RecordIOLibrary INSTANCE =
            (RecordIOLibrary) Native.loadLibrary("common_file_recordio_jna",
                                                 RecordIOLibrary.class);

        Pointer OpenRecordReader(String filename, int options);
        boolean ReadRecord(Pointer recordReader,
                           PointerByReference data,
                           IntByReference size);
        void CloseRecordReader(Pointer recordReader);

        Pointer OpenRecordWriter(String filename,
                                 int options,
                                 int compression_codec);
        boolean WriteRecord(Pointer recordWriter,
                            byte[] data,
                            int size);
        boolean FlushRecordWriter(Pointer recordWriter);
        void CloseRecordWriter(Pointer recordWriter);
    }

    public static void main(String[] args) {
        java.util.Random random = new java.util.Random(0);
        byte[][] test_data = new byte[10][];
        for (int i = 0; i < test_data.length; ++i) {
            int length =  random.nextInt(100) + 100;
            test_data[i] = new byte[length];
            random.nextBytes(test_data[i]);
        }

        java.io.File file = new java.io.File("/tmp/recordio_test1");
        file.delete();

        RecordWriter recordWriter = new RecordWriter(file.getPath(), 0, 0);
        for (int i = 0; i < test_data.length; ++i) {
            recordWriter.writeRecord(test_data[i]);
        }
        recordWriter.close();
        recordWriter = null;

        RecordReader recordReader =
            new RecordReader(file.getPath(),
                             RecordReader.RESUME_LAST_INCOMPLETE_BLOCK);
        int index = 0;
        byte[] data;
        while ((data = recordReader.readRecord()) != null) {
            assert index < test_data.length:
                "get more record than expected.";
            assert data.length == test_data[index].length:
                index + "nd record's length mismatched.";
            for (int i = 0; i < data.length; ++i) {
                assert data[i] == test_data[index][i]:
                    index + "nd record's " + i + "nd byte mismatched.";
            }
            System.out.println(index + "nd record matched.");
            ++index;
        }
        assert index == test_data.length:
            "get less record than expected.";
        recordReader.close();
        recordReader = null;
        System.out.println("All tests passed.");
    }
}

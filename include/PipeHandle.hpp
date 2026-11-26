class PipeHandle {
public:
    PipeHandle();
    
    // disable copy constructors
    PipeHandle(const PipeHandle&) = delete;
    PipeHandle& operator=(const PipeHandle&) = delete;

    ~PipeHandle();

    bool init();

    int get_read_end() const { return filedes[0]; }
    int get_write_end() const { return filedes[1]; }

    void close_read_end();
    void close_write_end();

private:
    int filedes[2] = {-1, -1};
};

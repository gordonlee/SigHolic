
// TODO: is this class really needed?
class INetwork {
public:
    virtual bool initialize() = 0;
    virtual void run() = 0;
    virtual void clean() = 0;
};
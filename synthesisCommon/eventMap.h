
class EventMap
{
public:
    void registerEvent(const string &event, const std::function<void(vector<string> &params)> &handler);
    void dispatchEvents(ml::UIConnection &ui) const;
    void dispatchEvents(const vector<string> &messages) const;

private:
    map< string, std::function<void(vector<string> &params)> > _handlers;
};
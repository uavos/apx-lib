#pragma once

namespace xdb {

class XdbItem
{
public:
    XdbItem *sibling() const { return _sibling; }
    void setSibling(XdbItem *s) { _sibling = s; }

private:
    XdbItem *_sibling{nullptr};
};

} // namespace xdb

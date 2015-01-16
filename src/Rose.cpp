#include "rapidcheck/detail/Rose.h"

#include "rapidcheck/Generator.h"

namespace rc {
namespace detail {

UnexpectedType::UnexpectedType(const std::type_info &expected,
                               const std::type_info &actual)
    : std::runtime_error(
        "Expected '" + demangle(expected.name())
        + "' but '" + demangle(actual.name()) + "' was requested")
    , m_expected(expected)
    , m_actual(actual) {}

RoseNode::RoseNode(RoseNode *parent)
    : m_parent(parent) {}

ValueDescription RoseNode::currentDescription(
    const gen::UntypedGenerator &generator)
{
    if (m_currentValue) {
        return m_currentValue.describe();
    } else if (m_acceptedValue) {
        return m_acceptedValue.describe();
    } else {
        ImplicitParam<param::CurrentNode> currentNode;
        currentNode.let(this);
        return generator.generateDescription();
    }
}

void RoseNode::acceptShrink()
{
    if (m_shrinkIterator) {
        m_shrinkIterator = nullptr;
        m_acceptedValue = std::move(m_currentValue);

        // Since we accepted a shrink value from the iterator, the values of the
        // children no longer have any relation to this value won't even be used
        // so we might as well get rid of them.
        m_children.clear();
    } else {
        assert(m_shrinkChild < m_children.size());
        m_children[m_shrinkChild].acceptShrink();
    }
}

RandomEngine::Atom RoseNode::atom()
{
    if (!m_hasAtom) {
        ImplicitParam<param::RandomEngine> randomEngine;
        m_atom = (*randomEngine)->nextAtom();
        m_hasAtom = true;
    }

    return m_atom;
}

std::vector<ValueDescription> RoseNode::example(
    const gen::UntypedGenerator &generator)
{
    std::vector<ValueDescription> example;
    // TODO this needs to be fixed
    // for (auto &child : m_children)
    //     example.push_back(child.currentDescription());
    return example;
}

RoseNode::RoseNode(RoseNode &&other)
    : m_parent(other.m_parent)
    , m_children(std::move(other.m_children))
    , m_shrinkChild(std::move(other.m_shrinkChild))
    , m_nextChild(other.m_nextChild)
    , m_hasAtom(other.m_hasAtom)
    , m_atom(other.m_atom)
    , m_shrinkIterator(std::move(other.m_shrinkIterator))
    , m_currentValue(std::move(other.m_currentValue))
    , m_acceptedValue(std::move(other.m_acceptedValue))
{
    adoptChildren();
}

RoseNode &RoseNode::operator=(RoseNode &&rhs)
{
    m_parent = rhs.m_parent;
    m_children = std::move(rhs.m_children);
    m_shrinkChild = std::move(rhs.m_shrinkChild);
    m_nextChild = rhs.m_nextChild;
    m_hasAtom = rhs.m_hasAtom;
    m_atom = rhs.m_atom;
    m_shrinkIterator = std::move(rhs.m_shrinkIterator);
    m_currentValue = std::move(rhs.m_currentValue);
    m_acceptedValue = std::move(rhs.m_acceptedValue);
    adoptChildren();
    return *this;
}

std::string RoseNode::debugDescription() const
{
    std::string desc;
    // TODO fix
    // if (!m_canonicalGenerator)
    //     desc += "<null>";
    // else
    //     desc += demangle(typeid(*m_canonicalGenerator).name());
    // desc += "[" + std::to_string(index()) + "]";
    return desc;
}

std::string RoseNode::debugPath() const
{
    if (m_parent == nullptr)
        return "/";
    else
        return m_parent->debugPath() + " " + debugDescription() + " /";
}

std::string RoseNode::debugIndexPath() const
{
    if (m_parent == nullptr)
        return "/";
    else
        return m_parent->debugIndexPath() + std::to_string(index()) + "/";
}

int RoseNode::depth() const
{
    if (m_parent == nullptr)
        return 0;
    else
        return m_parent->depth() + 1;
}

int RoseNode::index() const
{
    if (m_parent == nullptr) {
        return 0;
    } else {
        auto &parentChildren = m_parent->m_children;
        auto it = std::find_if(
            parentChildren.begin(), parentChildren.end(),
            [this] (const RoseNode &n) {
                return &n == this;
            });
        return it - parentChildren.begin();
    }
}

void RoseNode::adoptChildren()
{
    for (auto &child : m_children)
        child.m_parent = this;
}

} // namespace detail
} // namespace rc

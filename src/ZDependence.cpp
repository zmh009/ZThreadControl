#include "ZDependence.h"

ZDependence::ZDependence()
{
    ;
}

bool ZDependence::empty() const
{
    return mWorks.empty();
}

void ZDependence::push(const Fnt &f)
{
    mWorks.push_back(f);
}

void ZDependence::push(const ZRunnable &runnable)
{
    push(Fnt([&](){runnable.run();}));
}

void ZDependence::push(ZRunnable &runnable)
{
    push(Fnt([&](){runnable.run();}));
}

bool ZDependence::pushSub(const ZDependence &dependence)
{
    if (this == &dependence)
    {
        return false;
    }
    if (!valid(dependence))
    {
        return false;
    }

    mDependences.insert(&dependence);
    mDependences.insert(dependence.mDependences.begin(),dependence.mDependences.end());

    mSubs.push_back(&dependence);
    return true;
}

ZDependence::iterator ZDependence::begin() const
{
    return iterator(*this);
}

ZDependence::iterator ZDependence::end() const
{
    return iterator();
}

bool ZDependence::valid(const ZDependence &dependence) const
{
    if (this == &dependence)
    {
        return false;
    }
    // By determining whether they are in all the lower dependencies to determine whether there is a ring
    if (dependence.mDependences.find(this) != dependence.mDependences.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

ZDepIterator::ZDepIterator()
{
    mEnd = true;
}

ZDepIterator::ZDepIterator(const ZDependence &dependence)
{
    // Initializes the mDependencePool used by the store
    initSave(dependence);

    mEnd = mDependencePool.empty();
    mCurrent = mDependencePool.crbegin();
}

const list<Fnt> &ZDepIterator::operator *() const
{
    return *mCurrent;
}

const list<Fnt> *ZDepIterator::operator ->() const
{
    return & this->operator *();
}

ZDepIterator &ZDepIterator::operator ++()
{
    ++mCurrent;
    if (mCurrent == mDependencePool.crend())
    {
        mEnd = true;
    }
    return *this;
}

const ZDepIterator ZDepIterator::operator ++(int)
{
    ZDepIterator it(*this);
    this->operator ++();
    return it;
}

void ZDepIterator::initSave(const ZDependence &dependence)
{
    queue<const ZDependence *> deps1({&dependence});
    queue<const ZDependence *> deps2;
    list<Fnt> fnts;

    while (!deps1.empty() || !deps2.empty())
    {
        fnts.clear();
        while (!deps1.empty())
        {
            // Order to get Dependence class object
            const ZDependence *dep = deps1.front();

            // Store the child dependencies on another queue for use in the another iteration
            for (const ZDependence *each : dep->mSubs)
            {
                deps2.push(each);
            }

            // Stores the callable object of the currently dependent class
            for (const Fnt &each : dep->mWorks)
            {
                fnts.push_back(each);
            }

            // Cycles the next dependent class
            deps1.pop();
        }
        if (!fnts.empty())
        {
            // Equally dependent level of callable objects
            mDependencePool.push_back(fnts);
        }

        // The other loop is similar to the previous one
        fnts.clear();
        while (!deps2.empty())
        {
            const ZDependence *dep = deps2.front();
            for (const auto &each : dep->mSubs)
            {
                deps1.push(each);
            }
            for (const auto &each : dep->mWorks)
            {
                fnts.push_back(each);
            }
            deps2.pop();
        }
        if (!fnts.empty())
        {
            mDependencePool.push_back(fnts);
        }
    }
}

bool operator ==(const ZDepIterator &itor1, const ZDepIterator &itor2)
{
    return (itor1.mEnd == itor2.mEnd) || (itor1.mCurrent == itor2.mCurrent);
}

bool operator !=(const ZDepIterator &itor1, const ZDepIterator &itor2)
{
    return !operator ==(itor1,itor2);
}

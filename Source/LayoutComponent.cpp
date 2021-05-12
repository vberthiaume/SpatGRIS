#include "LayoutComponent.h"

#include "GrisLookAndFeel.h"

static auto const MAX_ELEM = [](auto const a, auto const b) {
    static_assert(std::is_same_v<decltype(a), decltype(b)>);
    return a < b ? b : a;
};

//==============================================================================
LayoutComponent::LayoutComponent(Orientation const orientation,
                                 bool const isHorizontalScrollable,
                                 bool const isVerticalScrollable,
                                 GrisLookAndFeel & lookAndFeel) noexcept
    : mOrientation(orientation)
    , mIsHorizontalScrollable(isHorizontalScrollable)
    , mIsVerticalScrollable(isVerticalScrollable)
{
    JUCE_ASSERT_MESSAGE_THREAD;

    mViewport.setScrollBarsShown(isVerticalScrollable, isHorizontalScrollable);
    mViewport.setScrollBarThickness(SCROLL_BAR_WIDTH);
    mViewport.getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId,
                                                 lookAndFeel.getScrollBarColour());
    mViewport.getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId,
                                               lookAndFeel.getScrollBarColour());
    mViewport.setViewedComponent(new juce::Component{}, true);
    addAndMakeVisible(mViewport);
}

//==============================================================================
LayoutComponent::Section & LayoutComponent::addSection(MinSizedComponent * component) noexcept
{
    JUCE_ASSERT_MESSAGE_THREAD;

    mSections.add(Section{});
    auto & section{ mSections.getReference(mSections.size() - 1) };
    section.mComponent = component;
    mViewport.getViewedComponent()->addAndMakeVisible(component);
    return section;
}

//==============================================================================
void LayoutComponent::clear()
{
    JUCE_ASSERT_MESSAGE_THREAD;
    mViewport.getViewedComponent()->removeAllChildren();
    mSections.clearQuick();
}

//==============================================================================
void LayoutComponent::paint(juce::Graphics & g)
{
    // for (auto const & section : mSections) {
    //    if (!section.mComponent) {
    //        continue;
    //    }
    //    auto const bounds{ section.mComponent->getBounds() };
    //    g.setColour(juce::Colours::blue.withAlpha(0.5f));
    //    g.fillRect(bounds);
    //    g.setColour(juce::Colours::red.withAlpha(0.5f));
    //    g.fillRect(bounds.reduced(2));
    //}
}

//==============================================================================
void LayoutComponent::resized()
{
    JUCE_ASSERT_MESSAGE_THREAD;

    auto const outerWidth{ getWidth() };
    auto const outerHeight{ getHeight() };

    auto const minInnerWidth{ getMinInnerWidth() };
    auto const minInnerHeight{ getMinInnerHeight() };

    auto const availableInnerWidth{ std::max(outerWidth, minInnerWidth) };
    auto const availableInnerHeight{ std::max(outerHeight, minInnerHeight) };

    auto const constantDimension{ mOrientation == Orientation::horizontal ? availableInnerHeight
                                                                          : availableInnerWidth };
    auto const dimensionToSplit{ mOrientation == Orientation::horizontal ? availableInnerWidth : availableInnerHeight };

    auto const minSpace{ mOrientation == Orientation::horizontal ? minInnerWidth : minInnerHeight };
    auto const spaceToShare{ std::max(dimensionToSplit - minSpace, 0) };

    auto const totalRelativeUnits{ std::transform_reduce(
        mSections.begin(),
        mSections.end(),
        0.0f,
        std::plus(),
        [](Section const & section) { return section.mRelativeSize; }) };
    auto const pixelsPerRelativeUnit{ totalRelativeUnits == 0.0f ? 0.0f : spaceToShare / totalRelativeUnits };

    if (mOrientation == Orientation::horizontal) {
        int offset{};
        for (auto & section : mSections) {
            auto const size{ section.computeComponentWidth(mOrientation, pixelsPerRelativeUnit) };
            if (section.mComponent) {
                section.mComponent->setBounds(offset + section.mLeftPadding,
                                              section.mTopPadding,
                                              size,
                                              constantDimension);
            }
            offset += section.mLeftPadding + size + section.mRightPadding;
        }
    } else {
        jassert(mOrientation == Orientation::vertical);
        int offset{};
        for (auto & section : mSections) {
            auto const size{ section.computeComponentHeight(mOrientation, pixelsPerRelativeUnit) };
            if (section.mComponent) {
                section.mComponent->setBounds(section.mLeftPadding,
                                              offset + section.mTopPadding,
                                              constantDimension,
                                              size);
            }
            offset += section.mTopPadding + size + section.mBottomPadding;
        }
    }

    mViewport.getViewedComponent()->setSize(availableInnerWidth, availableInnerHeight);
    mViewport.setSize(outerWidth, outerHeight);
}

//==============================================================================
int LayoutComponent::getMinWidth() const noexcept
{
    JUCE_ASSERT_MESSAGE_THREAD;

    if (mIsHorizontalScrollable) {
        return 0;
    }
    return getMinInnerWidth();
}

//==============================================================================
int LayoutComponent::getMinHeight() const noexcept
{
    JUCE_ASSERT_MESSAGE_THREAD;

    if (mIsVerticalScrollable) {
        return 0;
    }
    return getMinInnerHeight();
}

//==============================================================================
int LayoutComponent::getMinInnerWidth() const noexcept
{
    if (mOrientation == Orientation::horizontal) {
        return std::transform_reduce(mSections.begin(), mSections.end(), 0, std::plus(), [=](Section const & section) {
            return section.getMinSectionWidth(mOrientation);
        });
    }
    jassert(mOrientation == Orientation::vertical);
    return std::transform_reduce(mSections.begin(), mSections.end(), 0, MAX_ELEM, [=](Section const & section) {
        return section.getMinSectionWidth(mOrientation);
    });
}

//==============================================================================
int LayoutComponent::getMinInnerHeight() const noexcept
{
    if (mOrientation == Orientation::vertical) {
        return std::transform_reduce(mSections.begin(), mSections.end(), 0, std::plus(), [=](Section const & section) {
            return section.getMinSectionHeight(mOrientation);
        });
    }
    jassert(mOrientation == Orientation::horizontal);
    return std::transform_reduce(mSections.begin(), mSections.end(), 0, MAX_ELEM, [=](Section const & section) {
        return section.getMinSectionHeight(mOrientation);
    });
}

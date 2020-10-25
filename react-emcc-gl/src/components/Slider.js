import React from 'react'

const getPercentage = (value, max) => (100 * value) / max
const getValue = (percentage, max) => (max / 100) * percentage
const getLeft = percentage => `calc(${percentage}% - 5px)`


const Slot = React.forwardRef((prop, ref) => (
  <div {...prop} ref={ref} style={{
    position: 'relative',
    borderRadius: '15px',
    background: '#dddddd',
    height: '15px',
    paddingRight: '15px',
  }}>
    {prop.children}
  </div>
))

const Thumb = React.forwardRef((prop, ref) => {
  const { initialpercentage } = prop
  return (
    <div {...prop} ref={ref} style={{
      width: '25px',
      height: '25px',
      borderRadius: '15px',
      position: 'relative',
      top: '-5px',
      opacity: '0.5',
      background: '#823eb7',
      cursor: 'pointer',
      left: getLeft(initialpercentage)
    }} />
  )
})

const Slider = ({ initial, max, onChange }) => {

  const initialPercentage = getPercentage(initial, max)

  const slotRef = React.useRef()
  const thumbRef = React.useRef()
  const diff = React.useRef()

  const handleMouseMove = () => {
    let newX =
      window.event.clientX -
      diff.current -
      slotRef.current.getBoundingClientRect().left

    const end = slotRef.current.offsetWidth - thumbRef.current.offsetWidth

    const start = 0

    if (newX < start) {
      newX = 0
    }

    if (newX > end) {
      newX = end
    }

    const newPercentage = getPercentage(newX, end)
    const newValue = getValue(newPercentage, max)

    thumbRef.current.style.left = getLeft(newPercentage)

    onChange && onChange(newValue)
  }

  const handleMouseUp = () => {
    document.removeEventListener('mouseup', handleMouseUp)
    document.removeEventListener('mousemove', handleMouseMove)
  }

  const handleMouseDown = event => {
    diff.current =
      event.clientX - thumbRef.current.getBoundingClientRect().left
    document.addEventListener('mousemove', handleMouseMove)
    document.addEventListener('mouseup', handleMouseUp)
  }

  return (
    <Slot
      ref={slotRef}
    >
      <Thumb
        ref={thumbRef}
        onMouseDown={handleMouseDown}
        initialpercentage={initialPercentage}
      />
    </Slot>
  )
}

export default Slider
import React, { useEffect, useState } from 'react';
import './App.css';
import glModule from './generated/satori_em.js'
import Slider from './components/Slider';

const App = () => {
  const [module, setModule] = useState(null)

  useEffect(() => {
    const canvas = document.getElementById('canvas')
    glModule({ canvas }).then(
      Module => { setModule(Module) }
    )
  }, []);

  return (
    <div className="App">
      <div className="App-header">
        <canvas id="canvas" />
        <p>
          This is an <code>Emscripten-built C++ Opengl code</code>
        </p>
        <Slider
          initial={20}
          max={25}
        />
      </div>
    </div>
  );
}

export default App;

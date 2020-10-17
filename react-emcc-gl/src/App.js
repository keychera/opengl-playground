import React, { useEffect, useState } from 'react';
import './App.css';
import glModule from './hello_em.js'

const App = () => {
  const [module, setModule] = useState(null)

  useEffect(() => {
    const canvas = document.getElementById('canvas')
    glModule({ canvas }).then(
      Module => { setModule(Module) }
    )
  }, []);

  const toggleBg = () => {
    if (module) module._toggle_background_color()
  }

  return (
    <div className="App">
      <header className="App-header">
        <canvas id="canvas" />
        <p>
          This is an <code>Emscripten-built C++ Opengl code</code>
        </p>
        <button onClick={toggleBg}>
          toggle bg
        </button>
      </header>
    </div>
  );
}

export default App;

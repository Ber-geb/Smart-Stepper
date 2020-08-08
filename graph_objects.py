import pandas as pd
import plotly.graph_objects as go

df = pd.read_csv(filepath_or_buffer='1.csv')

fig = go.Figure(go.Scatter(x = df['X'], y = df['Y'],
                  name='Share Prices (in USD)'))

fig.update_layout(title='Apple Share Prices over time (2014)',
                   plot_bgcolor='rgb(230, 230,230)',
                   showlegend=True)

fig.show()